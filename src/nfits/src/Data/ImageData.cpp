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

namespace NFITS
{

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
void RawDataToPhysicalValuesTyped(std::vector<double>& out, const ImageMetadata& metadata, std::span<const std::byte> data)
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

bool RawDataToPhysicalValues(std::vector<double>& out, const ImageMetadata& metadata, std::span<const std::byte> data)
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

std::expected<ImageMetadata, bool> ParseImageMetadata(const HDU* pHDU)
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

    auto metadata = ImageMetadata{};
    metadata.bitpix = *bitpix;
    metadata.naxisns = naxisns;
    if (bZero) { metadata.bZero = *bZero; }
    if (bScale) { metadata.bScale = *bScale; }
    if (dataMin) { metadata.dataMin = *dataMin; }
    if (dataMax) { metadata.dataMax = *dataMax; }

    return metadata;
}

std::expected<std::vector<double>, bool> ReadDataAsPhysicalValues(const FITSFile* pFile, const HDU* pHDU, const ImageMetadata& metadata)
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

static constexpr std::size_t HISTOGRAM_NUM_BINS = 100;

void CalculateMinMax(PhysicalStats& physicalStats, std::span<const double> values)
{
    physicalStats.minMax = { std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest() };

    for (const auto& value : values)
    {
        // Skip over nan/infinity values
        if (!std::isfinite(value)) { continue; }

        physicalStats.minMax.first = std::min(physicalStats.minMax.first, value);
        physicalStats.minMax.second = std::max(physicalStats.minMax.second, value);
    }
}

void CalculateHistogram(PhysicalStats& physicalStats, std::span<const double> values)
{
    const auto rangeMin = physicalStats.minMax.first;
    const auto rangeMax = physicalStats.minMax.second;
    const auto rangeSpan = rangeMax - rangeMin;

    physicalStats.histogram = std::vector<std::size_t>(HISTOGRAM_NUM_BINS, 0);

    for (const auto& value : values)
    {
        // Skip over nan/infinity values
        if (!std::isfinite(value)) { continue; }

        const auto binIndex = static_cast<std::size_t>(((value - rangeMin) / rangeSpan) * (double)(HISTOGRAM_NUM_BINS - 1));

        physicalStats.histogram[binIndex]++;
    }
}

void CalculateHistogramCumulative(PhysicalStats& physicalStats)
{
    physicalStats.histogramCumulative = std::vector<std::size_t>(HISTOGRAM_NUM_BINS, 0);
    physicalStats.histogramCumulative.at(0) = physicalStats.histogram.at(0);

    for (std::size_t binIndex = 1; binIndex < HISTOGRAM_NUM_BINS; ++binIndex)
    {
        physicalStats.histogramCumulative.at(binIndex) = physicalStats.histogramCumulative.at(binIndex - 1) + physicalStats.histogram.at(binIndex);
    }
}

PhysicalStats CompilePhysicalStats(std::span<const double> values)
{
    PhysicalStats physicalStats{};

    CalculateMinMax(physicalStats, values);
    CalculateHistogram(physicalStats, values);
    CalculateHistogramCumulative(physicalStats);

    return physicalStats;
}

static inline uintmax_t GetNumSlices(const ImageMetadata& metadata)
{
    return metadata.naxisns.size() == 2 ? 1 :
    static_cast<uintmax_t>(std::accumulate(metadata.naxisns.cbegin() + 2, metadata.naxisns.cend(), 1, std::multiplies<>()));
}

static inline uintmax_t GetSliceDataSize(const ImageMetadata& metadata)
{
    return static_cast<uintmax_t>(metadata.naxisns.at(0) * metadata.naxisns.at(1));
}

static inline std::span<const double> GetSliceDataSpan(const ImageMetadata& metadata,  const uintmax_t& sliceIndex, std::span<const double> data)
{
    const auto sliceDataSize = GetSliceDataSize(metadata);

    return data.subspan(sliceIndex * sliceDataSize, sliceDataSize);
}

static inline uintmax_t GetNumSliceCubes(const ImageMetadata& metadata)
{
    return metadata.naxisns.size() <= 3 ? 1 :
       static_cast<uintmax_t>(std::accumulate(metadata.naxisns.cbegin() + 3, metadata.naxisns.cend(), 1, std::multiplies<>()));
}

static inline uintmax_t GetSliceCubeDataSize(const ImageMetadata& metadata)
{
    const auto sliceDataSize = GetSliceDataSize(metadata);;

    if (metadata.naxisns.size() == 2)
    {
        return sliceDataSize;
    }

    const auto slicesPerCube = static_cast<uintmax_t>(metadata.naxisns.at(2));

    return slicesPerCube * sliceDataSize;
}

static inline std::span<const double> GetSliceCubeDataSpan(const ImageMetadata& metadata,  const uintmax_t& sliceCubeIndex, std::span<const double> data)
{
    const auto sliceCubeDataSize = GetSliceCubeDataSize(metadata);

    return data.subspan(sliceCubeIndex * sliceCubeDataSize, sliceCubeDataSize);
}

bool ImageData::LoadFromFileBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    //
    // Sanity test that the HDU actually contains image data
    //
    if (pHDU->type != HDU::Type::Image)
    {
        std::cerr << "ImageData::LoadFromFileBlocking: HDU doesn't hold image data" << std::endl;
        return false;
    }

    //
    // Read metadata about the image from the HDU
    //
    auto metadata = ParseImageMetadata(pHDU);
    if (!metadata)
    {
        std::cerr << "ImageData::LoadFromFileBlocking: Failed to parse image metadata" << std::endl;
        return false;
    }

    //
    // Read the HDU data, transformed to physical values
    //
    auto physicalValues = ReadDataAsPhysicalValues(pFile, pHDU, *metadata);
    if (!physicalValues)
    {
        std::cerr << "ImageData::LoadFromFileBlocking: Failed to parse image data" << std::endl;
        return false;
    }

    //
    // Perform initial processing of the image's physical values
    //
    std::vector<PhysicalStats> slicePhysicalStats;

    for (uintmax_t sliceIndex = 0; sliceIndex < GetNumSlices(*metadata); ++sliceIndex)
    {
        const auto sliceData = GetSliceDataSpan(*metadata, sliceIndex, *physicalValues);
        slicePhysicalStats.push_back(CompilePhysicalStats(sliceData));
    }

    std::vector<PhysicalStats> sliceCubePhysicalStats;

    for (uintmax_t sliceCubeIndex = 0; sliceCubeIndex < GetNumSliceCubes(*metadata); ++sliceCubeIndex)
    {
        const auto sliceCubeData = GetSliceCubeDataSpan(*metadata, sliceCubeIndex, *physicalValues);
        sliceCubePhysicalStats.push_back(CompilePhysicalStats(sliceCubeData));
    }

    //
    // Successful load, set internal data
    //
    m_metadata = std::move(*metadata);
    m_physicalValues = std::move(*physicalValues);
    m_slicePhysicalStats = slicePhysicalStats;
    m_sliceCubePhysicalStats = sliceCubePhysicalStats;

    return true;
}

std::optional<uintmax_t> ImageData::GetSliceIndex(const ImageSlice& slice) const
{
    uintmax_t sliceIndex = 0;

    if (m_metadata.naxisns.size() > 2)
    {
        for (unsigned int n = 2; n < m_metadata.naxisns.size(); ++n)
        {
            int64_t axisValue = 0;

            const auto nValue = slice.axisValue.find(n + 1);
            if (nValue != slice.axisValue.cend())
            {
                axisValue = nValue->second;
            }

            uintmax_t prevAxesDataSpan = 1;
            for (unsigned int p = 2; p < n; ++p)
            {
                prevAxesDataSpan *= static_cast<uintmax_t>(m_metadata.naxisns.at(p));
            }

            sliceIndex += prevAxesDataSpan * static_cast<uintmax_t>(axisValue);
        }
    }

    if (sliceIndex >= m_slicePhysicalStats.size())
    {
        return std::nullopt;
    }

    return sliceIndex;
}

std::optional<uintmax_t> ImageData::GetSliceCubeIndex(const ImageSlice& slice) const
{
    // If <= 3 axes, there's only ever one slice cube
    if (m_metadata.naxisns.size() <= 3)
    {
        return 0;
    }

    const auto sliceIndex = GetSliceIndex(slice);
    if (!sliceIndex)
    {
        return std::nullopt;
    }

    const auto slicesPerCube = static_cast<uintmax_t>(m_metadata.naxisns.at(2));

    return *sliceIndex / slicesPerCube;
}

std::optional<std::span<const double>> ImageData::GetSlicePhysicalValues(const ImageSlice& slice) const
{
    const auto sliceIndex = GetSliceIndex(slice);
    if (!sliceIndex)
    {
        return std::nullopt;
    }

    return GetSliceDataSpan(m_metadata, *sliceIndex, m_physicalValues);
}

std::optional<PhysicalStats> ImageData::GetSlicePhysicalStats(const ImageSlice& slice) const
{
    const auto sliceIndex = GetSliceIndex(slice);
    if (!sliceIndex)
    {
        return std::nullopt;
    }

    return m_slicePhysicalStats.at(*sliceIndex);
}

std::optional<PhysicalStats> ImageData::GetSliceCubePhysicalStats(const ImageSlice& slice) const
{
    const auto sliceCubeIndex = GetSliceCubeIndex(slice);
    if (!sliceCubeIndex)
    {
        return std::nullopt;
    }

    return m_sliceCubePhysicalStats.at(*sliceCubeIndex);
}

}
