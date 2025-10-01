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

#include <numeric>

namespace NFITS
{

std::expected<Header, Error> ReadHeader(FITSBlockSource& blockSource, uintmax_t blockStartIndex)
{
    auto blockCount = blockSource.GetNumBlocks();
    if (!blockCount)
    {
        return std::unexpected(Error::Msg("ReadHeader: Unable to determine number of blocks in the source"));
    }

    if (blockStartIndex >= *blockCount)
    {
        return std::unexpected(Error::Msg("ReadHeader: Block index is out of bounds"));
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
            return std::unexpected(Error::Msg("ReadHeader: Failed to read block {} from the block source", blockIndex));
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
        return std::unexpected(Error::Msg("ReadHeader: Reached end of blocks but no END keyword found"));
    }

    return header;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize_Primary(const Header& header)
{
    // NAXIS (required)
    const auto naxisValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_NAXIS);
    if (!naxisValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Primary: NAXIS missing or not parseable")); }

    // No data, bail out early
    // TODO: Random Groups "(however, the random-groups structure described in Sect. 6 has
    //  NAXIS1 = 0, but will have data following the header if the other
    //  NAXISn keywords are non-zero)"
    if (*naxisValue == 0U) { return 0U; }

    // BITPIX (required)
    const auto bitpixValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BITPIX);
    if (!bitpixValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Primary: BITPIX missing or not parseable")); }

    // NAXISn (required)
    std::vector<int64_t> naxisNs;

    for (int64_t n = 0; n < *naxisValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_NAXIS, n + 1);
        const auto naxisnValue = header.GetFirstKeywordRecord_AsInteger(keywordName);
        if (!naxisnValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Primary: {} missing or not parseable", keywordName)); }

        naxisNs.push_back(*naxisnValue);
    }

    const auto naxisNsProduct = std::accumulate(naxisNs.cbegin(), naxisNs.cend(), int64_t{1}, std::multiplies<>());

    /**
     * [4.4.1.1. Primary header]
     * "Nbits = |BITPIX| × (NAXIS1 × NAXIS2 × · · · × NAXISm)"
     */
    const auto nbits = std::abs(*bitpixValue) * naxisNsProduct;

    const auto dataByteSize = static_cast<uintmax_t>(nbits / 8U);

    return dataByteSize;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize_Extension(const Header& header)
{
    // NAXIS (required)
    const auto naxisValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_NAXIS);
    if (!naxisValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Extension: NAXIS missing or not parseable")); }

    // No data, bail out early
    // TODO: Random Groups "(however, the random-groups structure described in Sect. 6 has
    //  NAXIS1 = 0, but will have data following the header if the other
    //  NAXISn keywords are non-zero)"
    if (*naxisValue == 0U) { return 0U; }

    // BITPIX (required)
    const auto bitpixValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BITPIX);
    if (!bitpixValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Extension: BITPIX missing or not parseable")); }

    // GCOUNT (required)
    const auto gCountValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_GCOUNT);
    if (!gCountValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Extension: GCOUNT missing or not parseable")); }

    // PCOUNT (required)
    const auto pCountValue = header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_PCOUNT);
    if (!pCountValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Extension: PCOUNT missing or not parseable")); }

    // NAXISn (required)
    std::vector<intmax_t> naxisNs;

    for (intmax_t n = 0; n < *naxisValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_NAXIS, n + 1);
        const auto naxisnValue = header.GetFirstKeywordRecord_AsInteger(keywordName);
        if (!naxisnValue) { return std::unexpected(Error::Msg("GetHDUDataByteSize_Extension: {} missing or not parseable", keywordName)); }

        naxisNs.push_back(*naxisnValue);
    }

    const auto naxisNsProduct = std::accumulate(naxisNs.cbegin(), naxisNs.cend(), int64_t{1}, std::multiplies<>());

    /**
     * [4.4.1.2. Conforming extensions]
     * Nbits = |BITPIX| × GCOUNT × (PCOUNT + NAXIS1 × NAXIS2 × · · · × NAXISm)
     */
    const auto nbits = std::abs(*bitpixValue) * *gCountValue * (*pCountValue + naxisNsProduct);

    const auto dataByteSize = static_cast<uintmax_t>(nbits / 8U);

    return dataByteSize;
}

std::expected<uintmax_t, Error> GetHDUDataByteSize(const Header& header, bool isPrimary)
{
    return isPrimary ? GetHDUDataByteSize_Primary(header) : GetHDUDataByteSize_Extension(header);
}

std::expected<HDU::Type, Error> GetHDUType(const Header& header)
{
    if (header.headerBlocks.empty())
    {
        return std::unexpected(Error::Msg("GetHDUType: Header has no associated header blocks"));
    }

    const auto& firstKeywordRecord = header.headerBlocks.at(0).keywordRecords.at(0);
    if (firstKeywordRecord.GetValidationError())
    {
        return std::unexpected(Error::Msg("GetHDUType: First keyword record has a validation error"));
    }

    const auto firstKeywordName = firstKeywordRecord.GetKeywordName();
    if (!firstKeywordName || !*firstKeywordName)
    {
        return std::unexpected(Error::Msg("GetHDUType: First keyword record has an invalid keyword name"));
    }

    if (*firstKeywordName == KEYWORD_NAME_SIMPLE)
    {
        return HDU::Type::Image;
    }

    // If not primary/simple HDU, the first keyword must be an XTENSION keyword
    if (*firstKeywordName != KEYWORD_NAME_XTENSION)
    {
        return std::unexpected(Error::Msg("GetHDUType: First keyword record has unexpected name: {}", **firstKeywordName));
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

    return std::unexpected(Error::Msg("GetHDUType: Unable to determine HDU type"));
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

    const auto dataByteSize = GetHDUDataByteSize(*header, isPrimary);
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
        .numDataBlocks = numDataBlocks,
        .dataByteSize = *dataByteSize
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
        const bool isPrimary = blockIndex == 0U;

        auto hdu = ReadHDU(blockSource, blockIndex, isPrimary);
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
