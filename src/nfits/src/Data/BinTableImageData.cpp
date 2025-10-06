/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Data/BinTableImageData.h>
#include <NFITS/Data/BinTableData.h>
#include <NFITS/Image/PhysicalStats.h>

#include <NFITS/HDU.h>
#include <NFITS/KeywordCommon.h>

#include "../Image/ImagePipeline.h"
#include "../Codec/Rice.h"
#include "../Util/Endianness.h"
#include "../Util/ImageUtilInternal.h"

#include <functional>
#include <cassert>

namespace NFITS
{

struct HDUBinTableImageMetadata
{
    std::string zCmpType;
    int64_t zBitpix;
    std::vector<int64_t> zNaxisns;
    std::vector<std::optional<int64_t>> zTilens;
    double zZero{0.0};
    double zScale{1.0};
};

std::expected<HDUBinTableImageMetadata, Error> ParseBinTableImageMetadata(const HDU* pHDU)
{
    //
    // Fetch required keyword records
    //
    const auto zImage = pHDU->header.GetFirstKeywordRecord_AsLogical(KEYWORD_NAME_ZIMAGE);
    if (!zImage) { return std::unexpected(Error::Msg("ZIMAGE missing or not parseable")); }
    const auto zImageValue = *zImage;

    const auto zCmpType = pHDU->header.GetFirstKeywordRecord_AsString(KEYWORD_NAME_ZCMPTYPE);
    if (!zCmpType) { return std::unexpected(Error::Msg("ZCMPTYPE missing or not parseable")); }
    const auto zCmpTypeValue = *zCmpType;

    const auto zBitpix = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_ZBITPIX);
    if (!zBitpix) { return std::unexpected(Error::Msg("ZBITPIX missing or not parseable")); }
    const auto zBitpixValue = *zBitpix;

    const auto zNaxis = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_ZNAXIS);
    if (!zNaxis) { return std::unexpected(Error::Msg("ZNAXIS missing or not parseable")); }
    const auto zNaxisValue = *zNaxis;

    // NAXISn (required)
    std::vector<int64_t> zNaxisns;

    for (int64_t n = 0; n < zNaxisValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_ZNAXIS, n + 1);
        const auto zNaxisnValue = pHDU->header.GetFirstKeywordRecord_AsInteger(keywordName);
        if (!zNaxisnValue) { return std::unexpected(Error::Msg("{} missing or not parseable", keywordName)); }

        zNaxisns.push_back(*zNaxisnValue);
    }

    //
    // Validate required keyword records
    //
    if (!zImageValue) { return std::unexpected(Error::Msg("ZIMAGE keyword not true")); }

    //
    // Fetch optional keyword records
    //
    std::vector<std::optional<int64_t>> zTilens;

    for (int64_t n = 0; n < zNaxisValue; ++n)
    {
        const auto keywordName = std::format("{}{}", KEYWORD_NAME_ZTILE, n + 1);
        const auto zTilenValue = pHDU->header.GetFirstKeywordRecord_AsInteger(keywordName);
        if (zTilenValue)
        {
            zTilens.emplace_back(*zTilenValue);
        }
        else
        {
            zTilens.emplace_back(std::nullopt);
        }
    }

    const auto zScale = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_ZSCALE);
    const auto zZero = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_ZZERO);

    return HDUBinTableImageMetadata{
        .zCmpType = zCmpTypeValue,
        .zBitpix = zBitpixValue,
        .zNaxisns = zNaxisns,
        .zTilens = zTilens,
        .zZero = zZero ? *zZero : 0.0,
        .zScale = zScale ? *zScale : 1.0
    };
}

template <typename T>
std::optional<T> ParseZVal(unsigned int n, const HDU* pHDU);

template <>
std::optional<int64_t> ParseZVal(unsigned int n, const HDU* pHDU)
{
    const auto zValKeyword = pHDU->header.GetFirstKeywordRecord_AsInteger(std::format("{}{}", KEYWORD_NAME_ZVAL, n));
    if (!zValKeyword)
    {
        return std::nullopt;
    }

    return *zValKeyword;
}

template <typename T>
std::optional<T> GetZVal(const std::string& zName, const HDU* pHDU)
{
    for (unsigned int n = 1; ; ++n)
    {
        const auto zNameKeyword = pHDU->header.GetFirstKeywordRecord_AsString(std::format("{}{}", KEYWORD_NAME_ZNAME, n));
        if (!zNameKeyword)
        {
            return std::nullopt;
        }

        if (*zNameKeyword != zName)
        {
            continue;
        }

        return ParseZVal<T>(n, pHDU);
    }

    assert(false);
    return std::nullopt;
}

std::expected<std::vector<double>, Error> Decompress_Rice1(const HDU* pHDU,
                                                           std::span<const std::byte> compressed,
                                                           const HDUBinTableImageMetadata& metadata)
{
    assert(!metadata.zNaxisns.empty());
    assert(metadata.zNaxisns.size() == metadata.zTilens.size());

    //
    // Parse tile size
    //

    // Default to ZNAXIS1 value
    auto tileSize = static_cast<std::size_t>(metadata.zNaxisns.at(0));

    // If ZTILE1 exists, use that tile size instead
    const auto zTilen = metadata.zTilens.at(0);
    if (zTilen) { tileSize = static_cast<std::size_t>(*zTilen); }

    //
    // Parse blocksize and bytepix from ZVALS
    //
    const auto blockSize = GetZVal<int64_t>("BLOCKSIZE", pHDU);
    if (!blockSize) { return std::unexpected(Error::Msg("Missing or bad BLOCKSIZE ZVAL")); }

    const auto bytepix = GetZVal<int64_t>("BYTEPIX", pHDU);
    if (!bytepix) { return std::unexpected(Error::Msg("Missing or bad BYTEPIX ZVAL")); }

    //
    // Decode data
    //
    RiceCodec rice(static_cast<unsigned int>(*blockSize));

    auto output = rice.Decompress(*bytepix, compressed, tileSize);
    if (!output)
    {
        return std::unexpected(output.error());
    }

    return output;
}

template <typename T, typename... Ts>
concept OneOf = (std::same_as<T, Ts> || ...);

/**
 * Takes in the bytes for a field in a bin table and interprets their content as variable array field
 * values, which contain two values: a number of elements and a heap byte offset, as either 32bit or 64 bit
 * signed ints.
 */
template <typename FieldType>
requires OneOf<FieldType, int32_t, int64_t>
std::pair<uint64_t, uint64_t> GetVariableArrayFieldValues(std::span<const std::byte> fieldBytes)
{
    // First X bytes contains number of elements
    auto numElementsVal = std::bit_cast<FieldType>(
        *reinterpret_cast<const std::array<std::byte, sizeof(FieldType)>*>(fieldBytes.subspan<0, sizeof(FieldType)>().data())
    );

    // Second X bytes contains heap byte offset
    auto heapByteOffsetVal = std::bit_cast<FieldType>(
        *reinterpret_cast<const std::array<std::byte, sizeof(FieldType)>*>(fieldBytes.subspan<sizeof(FieldType), sizeof(FieldType)>().data())
    );

    FixEndiannessPacked(numElementsVal);
    FixEndiannessPacked(heapByteOffsetVal);

    assert(numElementsVal >= 0);
    assert(heapByteOffsetVal >= 0);

    return std::make_pair(numElementsVal, heapByteOffsetVal);
}

std::expected<std::vector<double>, Error> ReadBinTableUncompressedImageData(const HDU* pHDU,
                                                                            const BinTableData* pBinTableData,
                                                                            const HDUBinTableImageMetadata& metadata)
{
    //
    // BinTable must have a COMPRESSED_DATA field
    //
    const auto compressedDataField = pBinTableData->GetFieldByName("COMPRESSED_DATA");
    if (!compressedDataField)
    {
        return std::unexpected(Error::Msg("Missing required keyword: COMPRESSED_DATA"));
    }

    //
    // Store a function to decompress data with, depending on compression type
    //
    using DecompressFunc = std::function<std::expected<std::vector<double>, Error>(
        const HDU* pHDU,
        std::span<const std::byte> compressed,
        const HDUBinTableImageMetadata& metadata)>;

    DecompressFunc decompressFunc{};

    if (metadata.zCmpType == "RICE_1")
    {
        decompressFunc = std::bind(&Decompress_Rice1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }
    else
    {
        return std::unexpected(Error::Msg("Unsupported compression type: {}", metadata.zCmpType));
    }

    // Type of the bintable field which contains dynamic array element count / heap offset
    const auto fieldType = compressedDataField->second.form.type;

    // Byte size of each dynamic array element
    const auto arrayElementByteSize = pBinTableData->GetVarArrayElementByteSize(compressedDataField->second);

    //
    // Read in each row of dynamic array data and decompress it
    //
    std::vector<double> output;

    for (std::size_t x = 0; x < pBinTableData->GetNumRows(); ++x)
    {
        //
        // Read the row field data, interpreted as element count + heap byte offset
        //
        const auto rowFieldBytes = pBinTableData->GetRowFieldBytes(x, compressedDataField->first);

        std::pair<uint64_t, uint64_t> fieldValues;

        switch (fieldType)
        {
            case BinFieldType::Array32Bit: { fieldValues = GetVariableArrayFieldValues<int32_t>(*rowFieldBytes); } break;
            case BinFieldType::Array64Bit: { fieldValues = GetVariableArrayFieldValues<int64_t>(*rowFieldBytes); } break;

            default:
            {
                return std::unexpected(Error::Msg("Unsupported COMPRESSED_DATA field type: {}", (unsigned int)fieldType));
            }
        }

        const auto numElements = fieldValues.first;
        const auto heapByteOffset = fieldValues.second;

        const auto heapByteSize = numElements * arrayElementByteSize;
        const auto heapSpan = std::span(pBinTableData->GetHeapBytes().data() + heapByteOffset, heapByteSize);

        //
        // Decompress the row's dynamic data
        //
        const auto decompressed = decompressFunc(pHDU, heapSpan, metadata);
        if (!decompressed)
        {
            return std::unexpected(decompressed.error());
        }

        //
        // Append the decompressed data to the final output
        //
        for (const auto& value : *decompressed)
        {
            output.push_back(value);
        }
    }

    return output;
}

std::expected<std::unique_ptr<BinTableImageData>, Error> LoadBinTableImageDataFromFileBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    //
    // Read the HDU's data as base BinTable data
    //
    const auto binTableData = LoadBinTableDataFromFileBlocking(pFile, pHDU);
    if (!binTableData)
    {
        return std::unexpected(binTableData.error());
    }

    //
    // Parse additional bintable image metadata
    //
    const auto metadata = ParseBinTableImageMetadata(pHDU);
    if (!metadata)
    {
        return std::unexpected(metadata.error());
    }

    //
    // Read the image data from the bintable and uncompress it
    //
    const auto imageValues = ReadBinTableUncompressedImageData(pHDU, binTableData->get(), *metadata);
    if (!imageValues)
    {
        return std::unexpected(imageValues.error());
    }

    //
    // Convert raw array values to physical values
    //
    std::vector<double> physicalValues = *imageValues;
    ApplyPhysicalValueTransform(physicalValues, metadata->zZero, metadata->zScale);
    // TODO: Apply BZERO/BSCALE?

    // TODO! ZBLANK

    //
    // Create an ImageData from the physical values
    //
    const auto sliceSpan = NaxisnsToSliceSpan(metadata->zNaxisns);
    if (!sliceSpan)
    {
        return std::unexpected(sliceSpan.error());
    }

    auto imageData = PhysicalValuesToImageData(std::move(physicalValues), *sliceSpan);
    if (!imageData)
    {
        return std::unexpected(imageData.error());
    }

    //
    // Create a BinTableImageData
    //
    return std::make_unique<BinTableImageData>(std::move(*(*imageData)));
}

BinTableImageData::BinTableImageData(ImageData&& imageData)
    : ImageData(std::move(imageData))
{

}

}
