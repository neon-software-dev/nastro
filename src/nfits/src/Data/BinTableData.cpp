/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Data/BinTableData.h>

#include <NFITS/HDU.h>
#include <NFITS/FITSFile.h>
#include <NFITS/FITSBlockSource.h>
#include <NFITS/KeywordCommon.h>

#include "../Parsing.h"

#include "../Util/Endianness.h"

#include <cassert>

namespace NFITS
{

// Checks for byte offset overlap (half-open)
inline bool Overlaps(uintmax_t a1, uintmax_t a2, uintmax_t b1, uintmax_t b2)
{
    return a1 < b2 && b1 < a2;
}

struct HDUBinTableMetadata
{
    int64_t bitpix{0};
    int64_t naxis1{0};
    int64_t naxis2{0};
    int64_t pcount{0};
    int64_t gcount{0};
    int64_t theap{0};
    std::vector<std::string> tformns;
    std::vector<std::optional<std::string>> ttypens;
};

inline bool IsArrayBinFieldType(BinFieldType fieldType)
{
    return fieldType == BinFieldType::Array32Bit || fieldType == BinFieldType::Array64Bit;
}

unsigned int GetBinFieldTypeBitSize(const BinFieldType& fieldType)
{
    switch (fieldType)
    {
        case BinFieldType::Logical: return 8;
        case BinFieldType::Bit: return 1;
        case BinFieldType::UnsignedByte: return 8;
        case BinFieldType::Integer16Bit: return 16;
        case BinFieldType::Integer32Bit: return 32;
        case BinFieldType::Integer64Bit: return 64;
        case BinFieldType::Character: return 8;
        case BinFieldType::FloatSinglePrecision: return 32;
        case BinFieldType::FloatDoublePrecision: return 64;
        case BinFieldType::ComplexSinglePrecision: return 64;
        case BinFieldType::ComplexDoublePrecision: return 128;
        case BinFieldType::Array32Bit: return 64;
        case BinFieldType::Array64Bit: return 128;
    }

    assert(false);
    return 0;
}

uintmax_t GetBinFieldByteSize(uintmax_t repeatCount, BinFieldType fieldType)
{
    // Special case for Bit field type, which should return the number of bytes required
    // to hold repeatCount bits
    if (fieldType == BinFieldType::Bit)
    {
        const auto numBits = repeatCount;
        const auto numBytes = repeatCount / 8;
        return (numBits % 8 == 0) ? numBytes : numBytes + 1;
    }

    // All other types are just the underlying data type byte size * repeatCount
    const auto numBits = GetBinFieldTypeBitSize(fieldType);
    assert(numBits % 8 == 0);

    return (numBits / 8) * repeatCount;
}

std::expected<uintmax_t, Error> GetRowByteSize(const std::vector<BinField>& fields)
{
    uintmax_t rowByteSize = 0;

    for (const auto& field : fields)
    {
        rowByteSize += GetBinFieldByteSize(field.form.repeatCount, field.form.type);
    }

    return rowByteSize;
}

void RawDataToRawRows(std::vector<BinTableRowBytes>& out, const uintmax_t& rowByteSize, std::span<const std::byte> data)
{
    assert(data.size() % rowByteSize == 0);

    for (std::size_t offset = 0; offset < data.size(); offset += rowByteSize)
    {
        const auto rowSpan = data.subspan(offset, rowByteSize);
        out.emplace_back(rowSpan.begin(), rowSpan.end());
    }
}

std::expected<std::pair<std::vector<BinTableRowBytes>, BinTableHeapBytes>, Error>
    ReadBinTableData(const FITSFile* pFile,
                     const HDU* pHDU,
                     const std::vector<BinField>& fields,
                     const HDUBinTableMetadata& metadata)
{
    auto blockSource = FITSBlockSource(pFile->GetByteSource());

    // Start & end HDU block indices containing all bintable data
    const auto dataBlockStartIndex = pHDU->GetDataBlockStartIndex();
    const auto dataBlockEndIndex = dataBlockStartIndex + pHDU->GetDataBlockCount();

    // Total byte size, table+supplemental, of the bintable
    const auto dataByteSize = pHDU->GetDataByteSize();

    // Byte size of the supplemental area (gap + heap)
    const auto supplementalByteSize = static_cast<uintmax_t>(metadata.pcount);

    // Byte size of the table itself
    const auto tableByteSize = dataByteSize - supplementalByteSize;

    // Byte size of the heap, within the supplemental area
    const auto heapByteSize = dataByteSize - static_cast<uintmax_t>(metadata.theap);

    // Byte size of each row in the table
    const auto rowByteSizeExpect = GetRowByteSize(fields);
    if (!rowByteSizeExpect)
    {
        return std::unexpected(rowByteSizeExpect.error());
    }
    const auto rowByteSize = *rowByteSizeExpect;

    if (tableByteSize % rowByteSize != 0)
    {
        return std::unexpected(Error::Msg("Table byte size isn't a clean multiple of row byte size"));
    }

    const auto numTableRows = tableByteSize / rowByteSize;

    //
    // Read data blocks into memory and store their data in rowBytes & heapBytes structures
    //
    std::vector<BinTableRowBytes> rowBytes;
    rowBytes.reserve(numTableRows);

    std::vector<std::byte> heapBytes;
    heapBytes.reserve(heapByteSize);

    // Byte offset to the start of the bintable data
    const uintmax_t dataByteStartOffset = dataBlockStartIndex * BLOCK_BYTE_SIZE.value;

    // Byte offsets defining the table's bytes
    const auto tableByteStartOffset = dataByteStartOffset;
    const auto tableByteEndOffset = tableByteStartOffset + tableByteSize;

    // Byte offsets defining the heap's bytes
    const auto heapByteStartOffset = dataByteStartOffset + static_cast<uintmax_t>(metadata.theap);
    const auto heapByteEndOffset = heapByteStartOffset + heapByteSize;

    BlockBytes blockBytes{};

    for (uintmax_t blockIndex = dataBlockStartIndex; blockIndex < dataBlockEndIndex; ++blockIndex)
    {
        if (!blockSource.ReadBlock(blockBytes, blockIndex))
        {
            return std::unexpected(Error::Msg("Failed to read next block of data from the file"));
        }

        const uintmax_t blockByteStartOffset = blockIndex * BLOCK_BYTE_SIZE.value;
        const uintmax_t blockByteEndOffset = blockByteStartOffset + BLOCK_BYTE_SIZE.value;

        // If this block contains table data, interpret and store it as table rows
        if (Overlaps(blockByteStartOffset, blockByteEndOffset, tableByteStartOffset, tableByteEndOffset))
        {
            const auto overlapStartByteOffset = std::max(blockByteStartOffset, tableByteStartOffset);
            const auto overlapEndByteOffset   = std::min(blockByteEndOffset, tableByteEndOffset);

            const auto blockTableDataSpan = std::span<const std::byte>(
                blockBytes.data() + (overlapStartByteOffset - blockByteStartOffset),
                overlapEndByteOffset - overlapStartByteOffset
            );

            RawDataToRawRows(rowBytes, rowByteSize, blockTableDataSpan);
        }

        // If this block contains heap data, read it and store it as raw heap data
        if (Overlaps(blockByteStartOffset, blockByteEndOffset, heapByteStartOffset, heapByteEndOffset))
        {
            const auto overlapStartByteOffset = std::max(blockByteStartOffset, heapByteStartOffset);
            const auto overlapEndByteOffset   = std::min(blockByteEndOffset, heapByteEndOffset);

            const auto blockHeapDataSpan = std::span<const std::byte>(
                blockBytes.data() + (overlapStartByteOffset - blockByteStartOffset),
                overlapEndByteOffset - overlapStartByteOffset
            );

            std::ranges::copy(blockHeapDataSpan, std::back_inserter(heapBytes));
        }
    }

    assert(rowBytes.size() == numTableRows);
    assert(heapBytes.size() == heapByteSize);

    return std::make_pair(std::move(rowBytes), std::move(heapBytes));
}

std::expected<HDUBinTableMetadata, Error> ParseBinTableMetadata(const HDU* pHDU)
{
    //
    // Fetch required keyword records
    //
    const auto bitpix = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BITPIX);
    if (!bitpix) { return std::unexpected(Error::Msg("BITPIX missing or not parseable")); }
    const auto bitpixValue = *bitpix;

    const auto naxis = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_NAXIS);
    if (!naxis) { return std::unexpected(Error::Msg("NAXIS missing or not parseable")); }
    const auto naxisValue = *naxis;

    const auto naxis1 = pHDU->header.GetFirstKeywordRecord_AsInteger(std::format("{}{}", KEYWORD_NAME_NAXIS, 1));
    if (!naxis1) { return std::unexpected(Error::Msg("NAXIS1 missing or not parseable")); }
    const auto naxis1Value = *naxis1;

    const auto naxis2 = pHDU->header.GetFirstKeywordRecord_AsInteger(std::format("{}{}", KEYWORD_NAME_NAXIS, 2));
    if (!naxis2) { return std::unexpected(Error::Msg("NAXIS2 missing or not parseable")); }
    const auto naxis2Value = *naxis2;

    const auto pcount = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_PCOUNT);
    if (!pcount) { return std::unexpected(Error::Msg("PCOUNT missing or not parseable")); }
    const auto pcountValue = *pcount;

    const auto gcount = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_GCOUNT);
    if (!gcount) { return std::unexpected(Error::Msg("GCOUNT missing or not parseable")); }
    const auto gcountValue = *gcount;

    const auto theap = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_THEAP);

    const auto tfields = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_TFIELDS);
    if (!tfields) { return std::unexpected(Error::Msg("TFIELDS missing or not parseable")); }
    const auto tfieldsValue = *tfields;

    std::vector<std::string> tformns;
    for (int64_t n = 1; n <= tfieldsValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_TFORM, n);
        const auto tformn = pHDU->header.GetFirstKeywordRecord_AsString(keywordName);
        if (!tformn) { return std::unexpected(Error::Msg("{} missing or not parseable", keywordName)); }
        tformns.push_back(*tformn);
    }

    std::vector<std::optional<std::string>> ttypens;
    for (int64_t n = 1; n <= tfieldsValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_TTYPE, n);
        const auto ttypen = pHDU->header.GetFirstKeywordRecord_AsString(keywordName);
        if (ttypen)
        {
            ttypens.emplace_back(*ttypen);
        }
        else
        {
            ttypens.emplace_back(std::nullopt);
        }
    }

    //
    // Validate required keyword records
    //
    if (naxisValue != 2) { return std::unexpected(Error::Msg("NAXIS for BINTABLE must have a value of 2")); }
    if (bitpixValue != 8) { return std::unexpected(Error::Msg("BITPIX for BINTABLE must have a value of 8")); }
    if (gcountValue != 1) { return std::unexpected(Error::Msg("GCOUNT for BINTABLE must have a value of 1")); }
    if (tfieldsValue < 0 || tfieldsValue > 999) { return std::unexpected(Error::Msg("TFIELDS for BINTABLE must be in the range [0,999]")); }

    //
    // Return result
    //
    const auto theapValue = theap ? *theap : naxis1Value * naxis2Value;

    return HDUBinTableMetadata{
        .bitpix = bitpixValue,
        .naxis1 = naxis1Value,
        .naxis2 = naxis2Value,
        .pcount = pcountValue,
        .gcount = gcountValue,
        .theap = theapValue,
        .tformns = tformns,
        .ttypens = ttypens
    };
}

std::expected<std::vector<BinFieldForm>, Error> ParseFieldForms(const HDUBinTableMetadata& metadata)
{
    std::vector<BinFieldForm> fieldForms;

    for (const auto& tformn : metadata.tformns)
    {
        const auto fieldForm = ParseBinTable_TFORMN(tformn);
        if (!fieldForm)
        {
            return std::unexpected(fieldForm.error());
        }
        fieldForms.push_back(*fieldForm);
    }

    return fieldForms;
}

std::expected<std::unique_ptr<BinTableData>, Error> LoadBinTableDataFromFileBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    //
    // Parse HDU metadata as BinTable metadata
    //
    const auto metadata = ParseBinTableMetadata(pHDU);
    if (!metadata)
    {
        return std::unexpected(Error::Msg("Failed to parse BinTable metadata"));
    }

    //
    // Compile fields from metadata
    //
    const auto fieldForms = ParseFieldForms(*metadata);
    if (!fieldForms)
    {
        return std::unexpected(Error::Msg("Failed to parse BinTable field forms"));
    }

    std::vector<BinField> fields;
    for (std::size_t x = 0; x < fieldForms->size(); ++x)
    {
        fields.push_back(BinField{.name = metadata->ttypens.at(x), .form = fieldForms->at(x)});
    }

    //
    // Read the BinTable data (table data + supplemental data) into memory
    //
    auto data = ReadBinTableData(pFile, pHDU, fields, *metadata);
    if (!data)
    {
        return std::unexpected(Error::Msg("Failed to read BinTable data"));
    }

    return std::make_unique<BinTableData>(fields, std::move(data->first), std::move(data->second));
}

BinTableData::BinTableData(std::vector<BinField> fields,
                           std::vector<BinTableRowBytes> rowBytes,
                           BinTableHeapBytes heapBytes)
    : m_fields(std::move(fields))
    , m_rowBytes(std::move(rowBytes))
    , m_heapBytes(std::move(heapBytes))
{

}

std::optional<std::span<const std::byte>> BinTableData::GetRowBytes(uintmax_t rowIndex) const
{
    if (rowIndex >= m_rowBytes.size())
    {
        return std::nullopt;
    }

    return m_rowBytes.at(rowIndex);
}

std::optional<std::span<const std::byte>> BinTableData::GetRowFieldBytes(uintmax_t rowIndex, uintmax_t fieldIndex) const
{
    if (fieldIndex >= m_fields.size())
    {
        return std::nullopt;
    }

    const auto field = m_fields.at(fieldIndex);
    const auto fieldByteSize = GetFieldByteSize(field);

    const auto rowBytes = GetRowBytes(rowIndex);
    if (!rowBytes)
    {
        return std::nullopt;
    }

    uintmax_t fieldByteOffset = 0;

    for (uintmax_t x = 0; x < fieldIndex; ++x)
    {
        fieldByteOffset += GetFieldByteSize(m_fields.at(x));
    }

    return std::span(rowBytes->data() + fieldByteOffset, fieldByteSize);
}

std::optional<std::pair<uintmax_t, BinField>> BinTableData::GetFieldByName(const std::string& fieldName) const
{
    for (std::size_t x = 0; x < m_fields.size(); ++x)
    {
        if (m_fields.at(x).name == fieldName)
        {
            return std::make_pair(x, m_fields.at(x));
        }
    }

    return std::nullopt;
}

uintmax_t BinTableData::GetFieldByteSize(const BinField& field) const
{
    return GetBinFieldByteSize(field.form.repeatCount, field.form.type);
}

uintmax_t BinTableData::GetVarArrayElementByteSize(const BinField& field) const
{
    assert(field.form.arrayType);

    return GetBinFieldByteSize(field.form.repeatCount, *field.form.arrayType);
}

}
