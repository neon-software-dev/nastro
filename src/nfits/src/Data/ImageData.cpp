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

#include "../Util/ImageUtilInternal.h"
#include "../Image/ImagePipeline.h"
#include "../WCS/WCSInternal.h"

#include <iostream>
#include <numeric>

namespace NFITS
{

struct HDUImageMetadata
{
    int64_t bitpix{0};
    int64_t naxis{0};
    std::vector<int64_t> naxisns;
    double bZero{0.0};
    double bScale{1.0};
    std::optional<int64_t> blank;
    std::optional<std::string> bUnit;
    std::optional<double> dataMin;
    std::optional<double> dataMax;
};

std::expected<HDUImageMetadata, Error> ParseImageMetadata(const HDU* pHDU)
{
    //
    // Required keywords
    //
    const auto bitpix = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BITPIX);
    if (!bitpix) { return std::unexpected(Error::Msg("Required keyword missing: BITPIX")); }

    const auto naxis = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_NAXIS);
    if (!naxis) { return std::unexpected(Error::Msg("Required keyword missing: NAXIS")); }

    std::vector<int64_t> naxisns;
    for (int64_t n = 1; n <= *naxis; ++n)
    {
        const auto naxisnStr = std::format("NAXIS{}", n);
        const auto naxisn = pHDU->header.GetFirstKeywordRecord_AsInteger(naxisnStr);
        if (!naxisn) { return std::unexpected(Error::Msg("Required keyword missing: {}", naxisnStr)); }
        naxisns.push_back(*naxisn);
    }

    //
    // Optional keywords
    //
    const auto bZero = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_BZERO);
    const auto bScale = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_BSCALE);
    const auto blank = pHDU->header.GetFirstKeywordRecord_AsInteger(KEYWORD_NAME_BLANK);
    const auto bUnit = pHDU->header.GetFirstKeywordRecord_AsString(KEYWORD_NAME_BUNIT);
    const auto dataMin = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_DATAMIN);
    const auto dataMax = pHDU->header.GetFirstKeywordRecord_AsReal(KEYWORD_NAME_DATAMAX);

    auto metadata = HDUImageMetadata{};
    metadata.bitpix = *bitpix;
    metadata.naxis = *naxis;
    metadata.naxisns = naxisns;
    if (bZero) { metadata.bZero = *bZero; }
    if (bScale) { metadata.bScale = *bScale; }
    if (blank) { metadata.blank = *blank; }
    if (bUnit) { metadata.bUnit = *bUnit; }
    if (dataMin) { metadata.dataMin = *dataMin; }
    if (dataMax) { metadata.dataMax = *dataMax; }

    return metadata;
}

std::expected<std::vector<double>, Error> ReadDataAsPhysicalValues(const FITSFile* pFile,
                                                                   const HDU* pHDU,
                                                                   const HDUImageMetadata& metadata)
{
    auto blockSource = FITSBlockSource(pFile->GetByteSource());

    const auto dataBlockStartIndex = pHDU->GetDataBlockStartIndex();
    const auto dataBlockEndIndex = dataBlockStartIndex + pHDU->GetDataBlockCount();
    const auto dataByteSize = pHDU->GetDataByteSize();

    const auto numPhysicalValues = std::accumulate(metadata.naxisns.cbegin(), metadata.naxisns.cend(), int64_t{1}, std::multiplies<>());

    std::vector<double> physicalValues;
    physicalValues.reserve(static_cast<std::size_t>(numPhysicalValues));

    uintmax_t numBytesRead = 0;
    BlockBytes blockBytes{};

    for (uintmax_t blockIndex = dataBlockStartIndex; blockIndex < dataBlockEndIndex; ++blockIndex)
    {
        const uintmax_t remainingDataBytes = dataByteSize - numBytesRead;

        if (!blockSource.ReadBlock(blockBytes, blockIndex))
        {
            return std::unexpected(Error::Msg("Failed to read data block"));
        }

        const uintmax_t blockDataBytes = std::min(remainingDataBytes, BLOCK_BYTE_SIZE.value);
        const auto blockDataSpan = std::span<std::byte>(blockBytes.data(), blockDataBytes);

        const auto blockPhysicalValues = RawImageDataToPhysicalValues(blockDataSpan,
                                                                      metadata.bitpix,
                                                                      metadata.bZero,
                                                                      metadata.bScale,
                                                                      metadata.blank);
        if (!blockPhysicalValues)
        {
            return std::unexpected(Error::Msg("Failed to convert data to physical values"));
        }

        for (const auto& physicalValue : *blockPhysicalValues)
        {
            physicalValues.push_back(physicalValue);
        }

        numBytesRead += blockDataBytes;
    }

    return physicalValues;
}

std::expected<std::unique_ptr<ImageData>, Error> LoadImageDataFromFileBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    //
    // Sanity test that the HDU actually contains image data
    //
    if (pHDU->type != HDU::Type::Image)
    {
        return std::unexpected(Error::Msg("HDU doesn't hold image data"));
    }

    //
    // Read metadata about the image from the HDU
    //
    const auto metadata = ParseImageMetadata(pHDU);
    if (!metadata)
    {
        return std::unexpected(metadata.error());
    }

    const auto wcsParams = ParseWCSParams(pHDU, metadata->naxis);
    if (!wcsParams)
    {
        return std::unexpected(wcsParams.error());
    }

    //
    // Read the HDU data, transformed to physical values
    //
    auto physicalValues = ReadDataAsPhysicalValues(pFile, pHDU, *metadata);
    if (!physicalValues)
    {
        return std::unexpected(physicalValues.error());
    }

    //
    // Create an ImageSliceSpan which defines the dimensionality of the image, built from naxisn metadata
    //
    const auto sliceSpan = NaxisnsToSliceSpan(metadata->naxisns);
    if (!sliceSpan)
    {
        return std::unexpected(sliceSpan.error());
    }

    //
    // Calculate slice statistics
    //
    const auto slicePhysicalStats = CalculateSlicePhysicalStats(*physicalValues, *sliceSpan);
    const auto sliceCubePhysicalStats = CalculateSliceCubePhysicalStats(*physicalValues, *sliceSpan);

    return std::make_unique<ImageData>(
        *sliceSpan,
        std::move(*physicalValues),
        slicePhysicalStats,
        sliceCubePhysicalStats,
        metadata->bUnit,
        *wcsParams
    );
}

ImageData::ImageData(ImageSliceSpan sliceSpan,
                     std::vector<double> physicalValues,
                     std::vector<PhysicalStats> slicePhysicalStats,
                     std::vector<PhysicalStats> sliceCubePhysicalStats,
                     std::optional<std::string> physicalUnit,
                     std::optional<WCSParams> wcsParams)
    : m_sliceSpan(std::move(sliceSpan))
    , m_physicalValues(std::move(physicalValues))
    , m_slicePhysicalStats(std::move(slicePhysicalStats))
    , m_sliceCubePhysicalStats(std::move(sliceCubePhysicalStats))
    , m_physicalUnit(std::move(physicalUnit))
    , m_wcsParams(std::move(wcsParams))
{

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
    if (m_sliceSpan.axes.size() <= 3)
    {
        return 0;
    }

    const auto sliceIndex = GetSliceIndex(sliceKey);
    if (!sliceIndex)
    {
        return std::nullopt;
    }

    const auto slicesPerCube = m_sliceSpan.axes.at(2);

    return *sliceIndex / static_cast<uint64_t>(slicesPerCube);
}

std::optional<ImageSlice> ImageData::GetImageSlice(const ImageSliceKey& sliceKey) const
{
    if (m_sliceSpan.axes.size() < 2)
    {
        return std::nullopt;
    }

    const auto sliceWidth = m_sliceSpan.axes.at(0);
    const auto sliceHeight = m_sliceSpan.axes.at(1);
    const auto sliceDataSize = static_cast<uintmax_t>(sliceWidth * sliceHeight);

    const auto sliceIndex = GetSliceIndex(sliceKey);
    const auto sliceCubeIndex = GetSliceCubeIndex(sliceKey);

    if (!sliceIndex || !sliceCubeIndex)
    {
        return std::nullopt;
    }

    const auto slicePhysicalValues = std::span<const double>(
        m_physicalValues.data() + (*sliceIndex * sliceDataSize),
        sliceDataSize
    );

    return ImageSlice{
        .width = static_cast<uint64_t>(sliceWidth),
        .height = static_cast<uint64_t>(sliceHeight),
        .physicalStats = m_slicePhysicalStats.at(*sliceIndex),
        .cubePhysicalStats = m_sliceCubePhysicalStats.at(*sliceCubeIndex),
        .physicalValues = slicePhysicalValues,
        .physicalUnit = m_physicalUnit,
        .wcsParams = m_wcsParams
    };
}

}
