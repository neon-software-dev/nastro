/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImagePipeline.h"

#include "../Util/Endianness.h"

#include <ranges>
#include <numeric>

namespace NFITS
{

inline double ApplyPhysicalValueTransform(double value, double bZero, double bScale)
{
    return bZero + (bScale * value);
}

void ApplyPhysicalValueTransform(std::vector<double>& values, double zero, double scale)
{
    std::ranges::transform(values, values.begin(), [&](const double& val){
        return ApplyPhysicalValueTransform(val, zero, scale);
    });
}

////

template<std::integral DataType>
std::vector<double> RawValuesToDoubles(std::span<const std::byte> data, const std::optional<int64_t>& blank)
{
    // Interpret the data as its underlying data type
    const auto typedData = std::span<const DataType>(
        reinterpret_cast<const DataType*>(data.data()),
        data.size() / sizeof(DataType)
    );

    std::vector<double> values;
    values.reserve(typedData.size());

    std::ranges::transform(typedData, std::back_inserter(values), [&](auto val){
        // Convert raw value to the endianness for this machine as needed
        FixEndiannessPacked(val);

        // Integral values have an optional "blank" value. If matched, return nan
        if (blank && (val == *blank))
        {
            return std::numeric_limits<double>::quiet_NaN();
        }

        // Interpret the raw value as a double
        return static_cast<double>(val);
    });

    return values;
}

template<std::floating_point DataType>
std::vector<double> RawValuesToDoubles(std::span<const std::byte> data)
{
    // Interpret the data as its underlying data type
    const auto typedData = std::span<const DataType>(
        reinterpret_cast<const DataType*>(data.data()),
        data.size() / sizeof(DataType)
    );

    std::vector<double> values;
    values.reserve(typedData.size());

    std::ranges::transform(typedData, std::back_inserter(values), [&](auto val){
        // Convert raw value to the endianness for this machine as needed
        FixEndiannessPacked(val);

        // Interpret the raw value as a double
        return static_cast<double>(val);
    });

    return values;
}

std::expected<std::vector<double>, Error> RawImageDataToPhysicalValues(std::span<const std::byte> data,
                                                                       int64_t bitpix,
                                                                       double bZero,
                                                                       double bScale,
                                                                       const std::optional<int64_t>& blank)
{
    // Convert the raw data to doubles
    std::vector<double> values;

    switch (bitpix)
    {
        case 8:     values = RawValuesToDoubles<uint8_t>(data, blank); break;
        case 16:    values = RawValuesToDoubles<int16_t>(data, blank); break;
        case 32:    values = RawValuesToDoubles<int32_t>(data, blank); break;
        case -32:   values = RawValuesToDoubles<float>(data); break;
        case -64:   values = RawValuesToDoubles<double>(data); break;
        default:
        {
            return std::unexpected(Error::Msg("Unsupported bitpix value: {}", bitpix));
        }
    }

    // Apply transform to convert from raw values to physical values
    ApplyPhysicalValueTransform(values, bZero, bScale);

    return values;
}

////

inline uintmax_t GetSliceDataSize(const ImageSliceSpan& sliceSpan)
{
    return sliceSpan.at(0) * sliceSpan.at(1);
}

inline std::span<const double> GetSliceDataSpan(const ImageSliceSpan& sliceSpan,
                                                const uintmax_t& sliceIndex,
                                                std::span<const double> data)
{
    const auto sliceDataSize = GetSliceDataSize(sliceSpan);

    return data.subspan(sliceIndex * sliceDataSize, sliceDataSize);
}

inline uint64_t GetNumSliceCubes(const ImageSliceSpan& sliceSpan)
{
    if (sliceSpan.empty())      { return 0U; }
    if (sliceSpan.size() <= 3)  { return 1U; }

    return std::accumulate(sliceSpan.cbegin() + 3U, sliceSpan.cend(), uint64_t{1U}, std::multiplies<>());
}

inline uintmax_t GetSliceCubeDataSize(const ImageSliceSpan& sliceSpan)
{
    const auto sliceDataSize = GetSliceDataSize(sliceSpan);

    if (sliceSpan.size() == 2)
    {
        return sliceDataSize;
    }

    const auto slicesPerCube = sliceSpan.at(2);

    return slicesPerCube * sliceDataSize;
}

inline std::span<const double> GetSliceCubeDataSpan(const ImageSliceSpan& sliceSpan,
                                                    const uintmax_t& sliceCubeIndex,
                                                    std::span<const double> data)
{
    const auto sliceCubeDataSize = GetSliceCubeDataSize(sliceSpan);

    return data.subspan(sliceCubeIndex * sliceCubeDataSize, sliceCubeDataSize);
}

std::expected<std::unique_ptr<ImageData>, Error>
PhysicalValuesToImageData(std::vector<double>&& physicalValues,
                          const std::optional<std::string>& physicalUnit,
                          const ImageSliceSpan& sliceSpan)
{
    if (sliceSpan.size() < 2)
    {
        return std::unexpected(Error::Msg("PhysicalValuesToImageData: Span must be at least 2-dimensional"));
    }

    const auto width = sliceSpan.at(0);
    const auto height = sliceSpan.at(1);
    const auto numSlices = GetNumSlicesInSpan(sliceSpan);
    const auto expectedValuesCount = numSlices * width * height;

    if (expectedValuesCount != physicalValues.size())
    {
        return std::unexpected(Error::Msg("PhysicalValuesToImageData: Physical values don't match span size"));
    }

    //
    // Calculate stats for the physical values in each slice
    //
    std::vector<PhysicalStats> slicePhysicalStats;

    for (uint64_t sliceIndex = 0; sliceIndex < GetNumSlicesInSpan(sliceSpan); ++sliceIndex)
    {
        const auto sliceData = GetSliceDataSpan(sliceSpan, sliceIndex, physicalValues);
        slicePhysicalStats.push_back(CompilePhysicalStats({sliceData}));
    }

    //
    // Calculate stats for the physical values in each slice cube
    //
    std::vector<PhysicalStats> sliceCubePhysicalStats;

    for (uint64_t sliceCubeIndex = 0; sliceCubeIndex < GetNumSliceCubes(sliceSpan); ++sliceCubeIndex)
    {
        const auto sliceCubeData = GetSliceCubeDataSpan(sliceSpan, sliceCubeIndex, physicalValues);
        sliceCubePhysicalStats.push_back(CompilePhysicalStats({sliceCubeData}));
    }

    return std::make_unique<ImageData>(
        sliceSpan,
        std::move(physicalValues),
        physicalUnit,
        std::move(slicePhysicalStats),
        std::move(sliceCubePhysicalStats)
    );
}

}