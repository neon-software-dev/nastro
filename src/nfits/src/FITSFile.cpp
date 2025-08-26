/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/FITSFile.h>
#include <NFITS/IFITSByteSource.h>
#include <NFITS/FITSBlockSource.h>
#include <NFITS/KeywordRecord.h>
#include <NFITS/KeywordCommon.h>

namespace NFITS
{

std::expected<Header, Error> ReadHeader(FITSBlockSource& blockSource, uintmax_t blockStartIndex)
{
    auto blockCount = blockSource.GetNumBlocks();
    if (!blockCount)
    {
        return std::unexpected(Error::Msg(ErrorType::General, "ReadHeader: Unable to determine number of blocks in the source"));
    }

    if (blockStartIndex >= *blockCount)
    {
        return std::unexpected(Error::Msg(ErrorType::General, "ReadHeader: Block index is out of bounds"));
    }

    //
    // Iterate over blocks starting at blockStartIndex, reading the block data as a HeaderBlock, until we find an END keyword
    //
    Header header{};
    BlockBytes blockBytes{};
    bool foundEndKeyword = false;

    for (uintmax_t blockIndex = blockStartIndex; blockIndex < *blockCount; ++blockIndex)
    {
        // Read the block's bytes from the source into memory
        if (!blockSource.ReadBlock(blockBytes, blockIndex))
        {
            return std::unexpected(Error::Msg(ErrorType::General, "ReadHeader: Failed to read block {} from the block source", blockIndex));
        }

        // Interpret the block's bytes as keyword records which fill a HeaderBlock
        HeaderBlock headerBlock{};

        for (unsigned int keywordRecordIndex = 0; keywordRecordIndex < KEYWORD_RECORDS_PER_HEADER_BLOCK; ++keywordRecordIndex)
        {
            const auto keywordRecordByteOffset = keywordRecordIndex * KEYWORD_RECORD_BYTE_SIZE;

            const auto keywordRecordSpan = KeywordRecordSpan{
                reinterpret_cast<char*>(blockBytes.data()) + keywordRecordByteOffset.value,
                KEYWORD_RECORD_BYTE_SIZE.value
            };

            const auto keywordRecord = KeywordRecord::FromRaw(keywordRecordSpan);

            headerBlock.keywordRecords[keywordRecordIndex] = keywordRecord;

            if (!keywordRecord.GetValidationError() && (*keywordRecord.GetKeywordName() == KEYWORD_NAME_END))
            {
                foundEndKeyword = true;
            }
        }

        header.headerBlocks.push_back(headerBlock);

        // If we reached an END keyword, stop reading further blocks
        if (foundEndKeyword)
        {
            break;
        }
    }

    if (!foundEndKeyword)
    {
        return std::unexpected(Error::Msg(ErrorType::General, "ReadHeader: Reached end of blocks but no END keyword found"));
    }

    return header;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize_Image(const Header& header)
{
    const auto bitpixValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BITPIX);
    if (!bitpixValue) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Image: Failed to get BITPIX value")); }

    const auto naxisValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_NAXIS);
    if (!naxisValue) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Image: Failed to get NAXIS value")); }

    // No image data
    if (*naxisValue == 0U)
    {
        return 0U;
    }

    std::vector<intmax_t> naxisNs;

    for (intmax_t n = 0; n < *naxisValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_NAXIS, n + 1);
        const auto naxisnValue = header.GetFirstKeywordRecord_AsInteger(keywordName);
        if (!naxisnValue) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Image: Failed to parse {} value to integer", keywordName)); }

        naxisNs.push_back(*naxisnValue);
    }

    // [4.4.1.1.]
    // "Nbits = |BITPIX| × (NAXIS1 × NAXIS2 × · · · × NAXISm)"
    auto nbits = std::abs(*bitpixValue);

    for (const auto& naxisn : naxisNs)
    {
        nbits *= naxisn;
    }

    const auto dataByteSize = static_cast<uintmax_t>(nbits / 8U);

    return dataByteSize;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize_Table(const Header& header)
{
    const auto naxis1Value = header.GetFirstKeywordRecord_AsInteger("NAXIS1");
    if (!naxis1Value) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Table: Failed to parse NAXIS1 value to integer")); }

    const auto naxis2Value = header.GetFirstKeywordRecord_AsInteger("NAXIS2");
    if (!naxis2Value) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Table: Failed to parse NAXIS2 value to integer")); }

    return *naxis1Value * *naxis2Value;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize_BinTable(const Header& header)
{
    const auto naxis1Value = header.GetFirstKeywordRecord_AsInteger("NAXIS1");
    if (!naxis1Value) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Table: Failed to parse NAXIS1 value to integer")); }

    const auto naxis2Value = header.GetFirstKeywordRecord_AsInteger("NAXIS2");
    if (!naxis2Value) { return std::unexpected(Error::Msg(ErrorType::Parse, "GetHDUDataByteSize_Table: Failed to parse NAXIS2 value to integer")); }

    return *naxis1Value * *naxis2Value;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize(const HDU::Type& type, const Header& header)
{
    std::expected<uintmax_t, Error> dataBlockCount{};

    switch (type)
    {
        case HDU::Type::Empty: dataBlockCount = 0U; break;
        case HDU::Type::Image: dataBlockCount = GetHDUDataByteSize_Image(header); break;
        case HDU::Type::Table: dataBlockCount = GetHDUDataByteSize_Table(header); break;
        case HDU::Type::BinTable: dataBlockCount = GetHDUDataByteSize_BinTable(header); break;
    }

    if (!dataBlockCount)
    {
        return std::unexpected(dataBlockCount.error());
    }

    return *dataBlockCount;
}

std::expected<HDU::Type, Error> GetHDUType(const Header& header)
{
    if (header.headerBlocks.empty())
    {
        return std::unexpected(Error::Msg(ErrorType::Validation, "GetHDUType: Header has no associated header blocks"));
    }

    const auto& firstKeywordRecord = header.headerBlocks.at(0).keywordRecords.at(0);
    if (firstKeywordRecord.GetValidationError())
    {
        return std::unexpected(Error::Msg(ErrorType::Validation, "GetHDUType: First keyword record has a validation error"));
    }

    const auto firstKeywordName = firstKeywordRecord.GetKeywordName();
    if (!firstKeywordName || !*firstKeywordName)
    {
        return std::unexpected(Error::Msg(ErrorType::Validation, "GetHDUType: First keyword record has an invalid keyword name"));
    }

    if (*firstKeywordName == KEYWORD_NAME_SIMPLE)
    {
        // Detect and special case handle empty primary HDUs
        const auto dataByteSize = GetHDUDataByteSize_Image(header);
        if (!dataByteSize)
        {
            return std::unexpected(Error::Msg(ErrorType::Validation, "GetHDUType: Failed to determine SIMPLE HDU byte size"));
        }

        return dataByteSize != 0U ? HDU::Type::Image : HDU::Type::Empty;
    }

    // If not primary/simple HDU, the first keyword must be an XTENSION keyword
    if (*firstKeywordName != KEYWORD_NAME_XTENSION)
    {
        return std::unexpected(Error::Msg(ErrorType::Validation, "GetHDUType: First keyword record has unexpected name: {}", **firstKeywordName));
    }

    const auto xtensionString = firstKeywordRecord.GetKeywordValue_AsString();

    // Check extension type
    if (xtensionString == KEYWORD_VALUE_XTENSION_IMAGE)
    {
        return HDU::Type::Image;
    }
    else if (xtensionString == KEYWORD_VALUE_XTENSION_TABLE)
    {
        return HDU::Type::Table;
    }
    else if (xtensionString == KEYWORD_VALUE_XTENSION_BINTABLE)
    {
        return HDU::Type::BinTable;
    }

    return std::unexpected(Error::Msg(ErrorType::Validation, "GetHDUType: Unable to determine HDU type"));
}

std::expected<HDU, Error> ReadHDU(FITSBlockSource& blockSource, uintmax_t blockStartIndex, bool isPrimary)
{
    const auto header = ReadHeader(blockSource, blockStartIndex);
    if (!header)
    {
        return std::unexpected(header.error());
    }

    const auto hduType = GetHDUType(*header);
    if (!hduType)
    {
        return std::unexpected(hduType.error());
    }

    const auto dataByteSize = GetHDUDataByteSize(*hduType, *header);
    if (!dataByteSize)
    {
        return std::unexpected(dataByteSize.error());
    }

    auto numDataBlocks = *dataByteSize / BLOCK_BYTE_SIZE.value;
    if (*dataByteSize % BLOCK_BYTE_SIZE.value != 0)
    {
        numDataBlocks++;
    }

    return HDU{
        .isPrimary = isPrimary,
        .type = *hduType,
        .blockStartIndex = blockStartIndex,
        .header = *header,
        .dataByteSize = *dataByteSize,
        .numDataBlocks = numDataBlocks
    };
}

std::expected<std::vector<HDU>, Error> ReadHDUS(FITSBlockSource& blockSource)
{
    std::vector<HDU> hdus;

    const auto blockCount = blockSource.GetNumBlocks();
    if (!blockCount)
    {
        return std::unexpected(blockCount.error());
    }

    uintmax_t blockIndex = 0;

    while (blockIndex < *blockCount)
    {
        auto hdu = ReadHDU(blockSource, blockIndex, blockIndex == 0U);
        if (!hdu)
        {
            return std::unexpected(hdu.error());
        }

        hdus.push_back(*hdu);

        blockIndex += hdu->GetTotalBlockCount();
    }

    return hdus;
}

std::expected<std::unique_ptr<FITSFile>, Error> FITSFile::OpenBlocking(std::unique_ptr<IFITSByteSource> pSource)
{
    auto blockSource = FITSBlockSource(pSource.get());

    const auto result = ReadHDUS(blockSource);
    if (!result)
    {
        return std::unexpected(result.error());
    }

    return std::make_unique<FITSFile>(Tag{}, std::move(pSource), *result);
}

FITSFile::FITSFile(Tag, std::unique_ptr<IFITSByteSource> pSource, std::vector<HDU> hdus)
    : m_pSource(std::move(pSource))
    , m_hdus(std::move(hdus))
{

}

FITSFile::~FITSFile() = default;

std::optional<const HDU*> FITSFile::GetHDU(uintmax_t index) const
{
    if (index >= m_hdus.size())
    {
        return std::nullopt;
    }

    return &m_hdus.at(index);
}

}
