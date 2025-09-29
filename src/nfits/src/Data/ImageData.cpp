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

#include <iostream>
#include <numeric>

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

std::expected<HDUImageMetadata, bool> ParseImageMetadata(const HDU* pHDU)
{
    //
    // Required keywords
    //
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

    //
    // Optional keywords
    //
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

std::expected<std::vector<double>, Error> ReadDataAsPhysicalValues(const FITSFile* pFile,
                                                                   const HDU* pHDU,
                                                                   const HDUImageMetadata& metadata)
{
    auto blockSource = FITSBlockSource(pFile->GetByteSource());

    const auto dataBlockStartIndex = pHDU->GetDataBlockStartIndex();
    const auto dataBlockEndIndex = dataBlockStartIndex + pHDU->GetDataBlockCount();
    const auto dataByteSize = pHDU->GetDataByteSize();

    const auto numPhysicalValues = std::accumulate(metadata.naxisns.cbegin(), metadata.naxisns.cend(), 1, std::multiplies<>());

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

        const auto blockPhysicalValues = RawImageDataToPhysicalValues(blockDataSpan, metadata.bitpix, metadata.bZero, metadata.bScale);
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
    auto metadata = ParseImageMetadata(pHDU);
    if (!metadata)
    {
        return std::unexpected(Error::Msg("Failed to parse image metadata"));
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
    // Perform initial processing of the image's physical values
    //
    return PhysicalValuesToImageData(std::move(*physicalValues), *sliceSpan);
}

ImageData::ImageData(ImageSliceSpan sliceSpan,
                     std::vector<double> physicalValues,
                     std::vector<PhysicalStats> slicePhysicalStats,
                     std::vector<PhysicalStats> sliceCubePhysicalStats)
    : m_sliceSpan(std::move(sliceSpan))
    , m_physicalValues(std::move(physicalValues))
    , m_slicePhysicalStats(std::move(slicePhysicalStats))
    , m_sliceCubePhysicalStats(std::move(sliceCubePhysicalStats))
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

    const auto sliceWidth = m_sliceSpan.at(0);
    const auto sliceHeight = m_sliceSpan.at(1);
    const auto sliceDataSize = sliceWidth * sliceHeight;

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
        .width = sliceWidth,
        .height = sliceHeight,
        .physicalStats = m_slicePhysicalStats.at(*sliceIndex),
        .cubePhysicalStats = m_sliceCubePhysicalStats.at(*sliceCubeIndex),
        .physicalValues = slicePhysicalValues
    };
}

}
