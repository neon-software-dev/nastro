/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Data/ImageData.h>
#include <NFITS/FITSFile.h>
#include <NFITS/HDU.h>
#include <NFITS/FITSBlockSource.h>
#include <NFITS/KeywordCommon.h>

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>

namespace NFITS
{

struct HDUImageMetadata
{
    int64_t bitpix{0};
    std::vector<int64_t> naxisns;
    double bZero{0.0};
    double bScale{1.0};
    std::optional<double> dataMin;
    std::optional<double> dataMax;
};

template <typename T>
inline constexpr void SwapEndiannessPacked(T& value) noexcept
{
    auto valueBytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(valueBytes);
    value = std::bit_cast<T>(valueBytes);
}

template <typename DataType>
double GetPhysicalValue(DataType dataValue, double bzero, double bscale)
{
    // FITS data is stored as big endian; swap endianness as needed for the current CPU
    if constexpr (std::endian::native != std::endian::big)
    {
        SwapEndiannessPacked(dataValue);
    }

    return bzero + (bscale * static_cast<double>(dataValue));
}

template <typename DataType>
void RawDataToPhysicalValuesTyped(std::vector<double>& out, const HDUImageMetadata& metadata, std::span<const std::byte> data)
{
    const auto typedData = std::span<const DataType>(
        reinterpret_cast<const DataType*>(data.data()),
        data.size() / sizeof(DataType)
    );

    for (const auto& typedVal : typedData)
    {
        out.push_back(GetPhysicalValue(typedVal, metadata.bZero, metadata.bScale));
    }
}

bool RawDataToPhysicalValues(std::vector<double>& out, const HDUImageMetadata& metadata, std::span<const std::byte> data)
{
    switch (metadata.bitpix)
    {
        case 8:     RawDataToPhysicalValuesTyped<uint8_t>(out, metadata, data); break;
        case 16:    RawDataToPhysicalValuesTyped<int16_t>(out, metadata, data); break;
        case 32:    RawDataToPhysicalValuesTyped<int32_t>(out, metadata, data); break;
        case -32:   RawDataToPhysicalValuesTyped<float>(out, metadata, data); break;
        case -64:   RawDataToPhysicalValuesTyped<double>(out, metadata, data); break;
        default:    return false;
    }

    return true;
}

std::expected<HDUImageMetadata, bool> ParseImageMetadata(const HDU* pHDU)
{
    const auto bitpix = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BITPIX);
    if (!bitpix) { return std::unexpected(false); }

    const auto naxis = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_NAXIS);
    if (!naxis) { return std::unexpected(false); }

    std::vector<int64_t> naxisns;
    for (int64_t n = 1; n <= *naxis; ++n)
    {
        const auto naxisn = pHDU->header.GetFirstKeywordRecord_AsInteger(std::format("NAXIS{}", n));
        if (!naxisn) { return std::unexpected(false); }
        naxisns.push_back(*naxisn);
    }

    const auto bZero = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_BZERO);
    const auto bScale = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_BSCALE);
    const auto dataMin = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_DATAMIN);
    const auto dataMax = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_DATAMAX);

    auto metadata = HDUImageMetadata{};
    metadata.bitpix = *bitpix;
    metadata.naxisns = naxisns;
    if (bZero) { metadata.bZero = *bZero; }
    if (bScale) { metadata.bScale = *bScale; }
    if (dataMin) { metadata.dataMin = *dataMin; }
    if (dataMax) { metadata.dataMax = *dataMax; }

    return metadata;
}

std::expected<std::vector<double>, bool> ReadDataAsPhysicalValues(const FITSFile* pFile, const HDU* pHDU, const HDUImageMetadata& metadata)
{
    auto blockSource = FITSBlockSource(pFile->GetByteSource());

    const auto dataBlockStartIndex = pHDU->GetDataBlockStartIndex();
    const auto dataBlockEndIndex = dataBlockStartIndex + pHDU->GetDataBlockCount();
    const auto dataByteSize = pHDU->GetDataByteSize();

    std::vector<double> physicalValues;

    const auto axesProduct = std::accumulate(metadata.naxisns.cbegin(), metadata.naxisns.cend(), 1, std::multiplies<>());
    physicalValues.reserve(static_cast<std::size_t>(axesProduct));

    uintmax_t numBytesRead = 0;
    BlockBytes blockBytes{};

    for (uintmax_t blockIndex = dataBlockStartIndex; blockIndex < dataBlockEndIndex; ++blockIndex)
    {
        const uintmax_t remainingDataBytes = dataByteSize - numBytesRead;

        if (!blockSource.ReadBlock(blockBytes, blockIndex))
        {
            std::cerr << "ImageData::LoadFromFileBlocking: Failed to read data block" << std::endl;
            return std::unexpected(false);
        }

        uintmax_t blockDataBytes = BLOCK_BYTE_SIZE.value;
        if (remainingDataBytes < blockDataBytes)
        {
            blockDataBytes = remainingDataBytes;
        }

        const auto blockDataSpan = std::span<std::byte>(blockBytes.data(), blockDataBytes);
        RawDataToPhysicalValues(physicalValues, metadata, blockDataSpan);

        numBytesRead += blockDataBytes;
    }

    return physicalValues;
}

static inline uintmax_t GetSliceDataSize(const ImageSliceSpan& sliceSpan)
{
    return sliceSpan.at(0) * sliceSpan.at(1);
}

static inline std::span<const double> GetSliceDataSpan(const ImageSliceSpan& sliceSpan, const uintmax_t& sliceIndex, std::span<const double> data)
{
    const auto sliceDataSize = GetSliceDataSize(sliceSpan);

    return data.subspan(sliceIndex * sliceDataSize, sliceDataSize);
}

static inline uint64_t GetNumSliceCubes(const ImageSliceSpan& sliceSpan)
{
    return sliceSpan.size() <= 3 ? 1U :
       std::accumulate(sliceSpan.cbegin() + 3U, sliceSpan.cend(), 1U, std::multiplies<>());
}

static inline uintmax_t GetSliceCubeDataSize(const ImageSliceSpan& sliceSpan)
{
    const auto sliceDataSize = GetSliceDataSize(sliceSpan);

    if (sliceSpan.size() == 2)
    {
        return sliceDataSize;
    }

    const auto slicesPerCube = sliceSpan.at(2);

    return slicesPerCube * sliceDataSize;
}

static inline std::span<const double> GetSliceCubeDataSpan(const ImageSliceSpan& sliceSpan,  const uintmax_t& sliceCubeIndex, std::span<const double> data)
{
    const auto sliceCubeDataSize = GetSliceCubeDataSize(sliceSpan);

    return data.subspan(sliceCubeIndex * sliceCubeDataSize, sliceCubeDataSize);
}

std::expected<std::unique_ptr<ImageData>, Error> ImageData::LoadFromFileBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    //
    // Sanity test that the HDU actually contains image data
    //
    if (pHDU->type != HDU::Type::Image)
    {
        return std::unexpected(Error::Msg(ErrorType::General, "HDU doesn't hold image data"));
    }

    //
    // Read metadata about the image from the HDU
    //
    auto metadata = ParseImageMetadata(pHDU);
    if (!metadata)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "Failed to parse image metadata"));
    }

    //
    // Read the HDU data, transformed to physical values
    //
    auto physicalValues = ReadDataAsPhysicalValues(pFile, pHDU, *metadata);
    if (!physicalValues)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "Failed to parse image data as physical values"));
    }

    ImageSliceSpan sliceSpan{};

    for (const auto& naxisn : metadata->naxisns)
    {
        if (naxisn < 0)
        {
            return std::unexpected(Error::Msg(ErrorType::Validation, "Out of bounds naxisn value: {}", naxisn));
        }

        sliceSpan.push_back(static_cast<uint64_t>(naxisn));
    }

    //
    // Perform initial processing of the image's physical values
    //
    std::vector<PhysicalStats> slicePhysicalStats;

    for (uint64_t sliceIndex = 0; sliceIndex < GetNumSlicesInSpan(sliceSpan); ++sliceIndex)
    {
        const auto sliceData = GetSliceDataSpan(sliceSpan, sliceIndex, *physicalValues);
        slicePhysicalStats.push_back(CompilePhysicalStats({sliceData}));
    }

    std::vector<PhysicalStats> sliceCubePhysicalStats;

    for (uint64_t sliceCubeIndex = 0; sliceCubeIndex < GetNumSliceCubes(sliceSpan); ++sliceCubeIndex)
    {
        const auto sliceCubeData = GetSliceCubeDataSpan(sliceSpan, sliceCubeIndex, *physicalValues);
        sliceCubePhysicalStats.push_back(CompilePhysicalStats({sliceCubeData}));
    }

    //
    // Successful load
    //
    auto pImageData = std::make_unique<ImageData>();
    pImageData->m_sliceSpan = sliceSpan;
    pImageData->m_physicalValues = std::move(*physicalValues);
    pImageData->m_slicePhysicalStats = slicePhysicalStats;
    pImageData->m_sliceCubePhysicalStats = sliceCubePhysicalStats;

    return pImageData;
}

std::optional<uintmax_t> ImageData::GetSliceIndex(const ImageSliceKey& sliceKey) const
{
    const auto sliceIndex = SliceKeyToLinearIndex(m_sliceSpan, sliceKey);
    if (!sliceIndex)
    {
        std::cerr << "GetSliceIndex: Error: " << sliceIndex.error().msg << std::endl;
        return std::nullopt;
    }

    if (*sliceIndex >= m_slicePhysicalStats.size())
    {
        std::cerr << "GetSliceIndex: Out of bounds slice key" << std::endl;
        return std::nullopt;
    }

    return *sliceIndex;
}

std::optional<uintmax_t> ImageData::GetSliceCubeIndex(const ImageSliceKey& sliceKey) const
{
    // If <= 3 axes, there's only ever one slice cube
    if (m_sliceSpan.size() <= 3)
    {
        return 0;
    }

    const auto sliceIndex = GetSliceIndex(sliceKey);
    if (!sliceIndex)
    {
        return std::nullopt;
    }

    const auto slicesPerCube = m_sliceSpan.at(2);

    return *sliceIndex / slicesPerCube;
}

std::optional<ImageSlice> ImageData::GetImageSlice(const ImageSliceKey& sliceKey) const
{
    if (m_sliceSpan.size() < 2)
    {
        return std::nullopt;
    }

    const auto sliceIndex = GetSliceIndex(sliceKey);
    const auto sliceCubeIndex = GetSliceCubeIndex(sliceKey);

    if (!sliceIndex || !sliceCubeIndex)
    {
        return std::nullopt;
    }

    return ImageSlice{
        .width = m_sliceSpan.at(0),
        .height = m_sliceSpan.at(1),
        .physicalStats = m_slicePhysicalStats.at(*sliceIndex),
        .cubePhysicalStats = m_sliceCubePhysicalStats.at(*sliceCubeIndex),
        .physicalValues = GetSliceDataSpan(m_sliceSpan, *sliceIndex, m_physicalValues)
    };
}

}
