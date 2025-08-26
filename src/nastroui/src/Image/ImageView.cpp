/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageView.h"

#include "../ColorMaps/colormap.h"
#include "../ColorMaps/cet_lut.h"

#include <NFITS/Data/ImageData.h>
#include <NFITS/Util/ImageUtil.h>

#include <iostream>
#include <algorithm>

namespace Nastro
{

std::pair<double, double> ChoosePhysicalValueRange(const ImageRenderParams& params, const NFITS::PhysicalStats& physicalStats)
{
    switch (params.scalingRange)
    {
        case ScalingRange::Full: return {physicalStats.minMax.first, physicalStats.minMax.second};
        case ScalingRange::p99: return NFITS::CalculatePercentileRange(physicalStats, 0.99f);
        case ScalingRange::p95: return NFITS::CalculatePercentileRange(physicalStats, 0.95f);
        case ScalingRange::Custom:
        {
            const double min = params.customScalingRangeMin ? *params.customScalingRangeMin : physicalStats.minMax.first;
            const double max = params.customScalingRangeMax ? *params.customScalingRangeMax : physicalStats.minMax.second;

            return {min, max};
        }
    }

    assert(false);
    return {0.0, 0.0};
}

std::expected<QImage, bool> PhysicalValuesToQImage(const NFITS::ImageSlice& slice,
                                                   const ImageRenderParams& params,
                                                   const NFITS::ImageData* pImageData)
{
    const auto& metadata = pImageData->GetMetadata();
    const auto& imageWidth = metadata.naxisns.at(0);
    const auto& imageHeight = metadata.naxisns.at(1);

    //
    // Fetch the physical stats of the data to be rendered
    //
    std::optional<NFITS::PhysicalStats> physicalStats;

    switch (params.scalingMode)
    {
        case ScalingMode::PerImage: { physicalStats = pImageData->GetSlicePhysicalStats(slice); } break;
        case ScalingMode::PerCube:  { physicalStats = pImageData->GetSliceCubePhysicalStats(slice); } break;
    }

    if (!physicalStats)
    {
        std::cerr << "RenderImageData: Failed to determine slice physical stats" << std::endl;
        return std::unexpected(false);
    }

    //
    // Determine min/max physical values to use
    //
    const auto chosenRange = ChoosePhysicalValueRange(params, *physicalStats);
    const auto physicalValueMin = chosenRange.first;
    const auto physicalValueMax = chosenRange.second;
    const auto physicalValueRange = physicalValueMax - physicalValueMin;

    //
    // Fetch a span over the slice's physical values
    //
    const auto slicePhysicalValues = pImageData->GetSlicePhysicalValues(slice);
    if (!slicePhysicalValues)
    {
        std::cerr << "RenderImageData: Invalid slice provided" << std::endl;
        return std::unexpected(false);
    }

    //
    // Create a QImage and fill it with interpreted image data
    //
    auto qImage = QImage(static_cast<int>(imageWidth), static_cast<int>(imageHeight), QImage::Format::Format_RGB888);

    for (int64_t y = 0; y < imageHeight; ++y)
    {
        // FITS convention is for images to be stored bottom to top, while Qt images are stored
        // top to bottom, so some vertical swapping here to fill the Qt image from bottom up
        // as we read FITS image data from top down
        uchar* pScanline = qImage.scanLine(static_cast<int>(imageHeight - y - 1));

        for (int64_t x = 0; x < imageWidth; ++x)
        {
            const auto physicalValueIndex = static_cast<uintmax_t>(x + (y * imageWidth));

            const double physicalValue = std::clamp((*slicePhysicalValues)[physicalValueIndex], physicalValueMin, physicalValueMax);

            double norm = (physicalValue - physicalValueMin) / physicalValueRange;

            // Force clamp norm to be within [0.0..1.0]. Shouldn't ever be outside this range though, unless
            // floating imprecision when calculating norm caused it to drift ever so slightly out of range
            norm = std::clamp(norm, 0.0, 1.0);

            //
            // Apply transfer function to map normalized value to display value
            //
            double displayValue = 0.0; // [0..1]

            switch (params.transferFunction)
            {
                case TransferFunction::Linear:  displayValue = norm; break;
                case TransferFunction::Log:     displayValue = std::log1p(norm * (params.logTransferBase - 1.0)) / std::log(params.logTransferBase); break;
                case TransferFunction::Sqrt:    displayValue = std::pow(norm, 1.0 / 2.0); break;
                case TransferFunction::Square:  displayValue = std::pow(norm, 1.0 / 0.5); break;
                case TransferFunction::Asinh:   displayValue = asinh(params.asinhTransferScale * norm) / asinh(params.asinhTransferScale); break;
            }

            //
            // Apply color map function to map display value to RGB color
            //
            std::optional<std::array<double, 3>> colorPercentages; // Filled in with color percentages below if a pre-defined colormap is configured
            std::optional<const std::array<uint8_t, 768>*> cet_lut; // Filled in with a LUT below if a specific CET is configured

            switch (params.colorMap)
            {
                case ColorMap::Fire:    { colorPercentages = {0.0}; colormap::ramp::fire(displayValue, colorPercentages->data()); } break;
                case ColorMap::Ocean:   { colorPercentages = {0.0}; colormap::ramp::ocean(displayValue, colorPercentages->data()); } break;
                case ColorMap::Ice:     { colorPercentages = {0.0}; colormap::ramp::ice(displayValue, colorPercentages->data()); } break;

                case ColorMap::CET_L01: { cet_lut = &CET::L01; } break;
                case ColorMap::CET_L02: { cet_lut = &CET::L02; } break;
                case ColorMap::CET_L03: { cet_lut = &CET::L03; } break;
                case ColorMap::CET_L04: { cet_lut = &CET::L04; } break;
                case ColorMap::CET_L05: { cet_lut = &CET::L05; } break;
                case ColorMap::CET_L06: { cet_lut = &CET::L06; } break;
                case ColorMap::CET_L07: { cet_lut = &CET::L07; } break;
                case ColorMap::CET_L08: { cet_lut = &CET::L08; } break;
                case ColorMap::CET_L09: { cet_lut = &CET::L09; } break;
                case ColorMap::CET_L10: { cet_lut = &CET::L10; } break;
                case ColorMap::CET_L11: { cet_lut = &CET::L11; } break;
                case ColorMap::CET_L12: { cet_lut = &CET::L12; } break;
                case ColorMap::CET_L13: { cet_lut = &CET::L13; } break;
                case ColorMap::CET_L14: { cet_lut = &CET::L14; } break;
                case ColorMap::CET_L15: { cet_lut = &CET::L15; } break;
                case ColorMap::CET_L16: { cet_lut = &CET::L16; } break;
                case ColorMap::CET_L17: { cet_lut = &CET::L17; } break;
                case ColorMap::CET_L18: { cet_lut = &CET::L18; } break;
                case ColorMap::CET_L19: { cet_lut = &CET::L19; } break;
                case ColorMap::CET_L20: { cet_lut = &CET::L20; } break;

                case ColorMap::CET_D01: { cet_lut = &CET::D01; } break;
                case ColorMap::CET_D01A: { cet_lut = &CET::D01A; } break;
                case ColorMap::CET_D02: { cet_lut = &CET::D02; } break;
                case ColorMap::CET_D03: { cet_lut = &CET::D03; } break;
                case ColorMap::CET_D04: { cet_lut = &CET::D04; } break;
                case ColorMap::CET_D06: { cet_lut = &CET::D06; } break;
                case ColorMap::CET_D07: { cet_lut = &CET::D07; } break;
                case ColorMap::CET_D08: { cet_lut = &CET::D08; } break;
                case ColorMap::CET_D09: { cet_lut = &CET::D09; } break;
                case ColorMap::CET_D10: { cet_lut = &CET::D10; } break;
                case ColorMap::CET_D13: { cet_lut = &CET::D13; } break;
                case ColorMap::CET_R3: { cet_lut = &CET::R3; } break;

                case ColorMap::CET_R1: { cet_lut = &CET::R1; } break;
                case ColorMap::CET_R2: { cet_lut = &CET::R2; } break;
                case ColorMap::CET_R4: { cet_lut = &CET::R4; } break;
            }

            std::array<unsigned char, 3> rgbColors{0};

            if (colorPercentages)
            {
                // Directly transform RGB color percentages to RGB color bytes
                std::ranges::transform(*colorPercentages, rgbColors.begin(), [](const auto& percentage){
                    return static_cast<unsigned char>(percentage * 255.0);
                });
            }
            else if (cet_lut)
            {
                // Use displayValuePercent to index into the LUT and copy RGB color bytes from it
                const std::size_t offset = static_cast<unsigned char>(displayValue * 255.0) * 3;
                std::copy_n((*cet_lut)->cbegin() + offset, 3, rgbColors.begin());
            }

            //
            // Write final pixel values to the QImage
            //
            for (unsigned int pos = 0; pos < rgbColors.size(); ++pos)
            {
                pScanline[(x * 3) + pos] = rgbColors[pos];
            }
        }
    }

    return qImage;
}

void ApplyPostProcessing(QImage& qImage, const ImageRenderParams& params)
{
    for (int y = 0; y < qImage.height(); ++y)
    {
        auto pScanline = qImage.scanLine(y);

        for (int x = 0; x < qImage.width(); ++x)
        {
            if (params.invertColors)
            {
                for (int v = 0; v < 3; ++v)
                {
                    pScanline[(x * 3) + v] = 255 - pScanline[(x * 3) + v];
                }
            }
        }
    }
}

std::expected<QImage, bool> RenderImageData(const NFITS::ImageSlice& slice, const ImageRenderParams& params, const NFITS::ImageData* pImageData)
{
    //
    // Process the slice's physical values into a rendered image
    //
    auto sliceImage = PhysicalValuesToQImage(slice, params, pImageData);
    if (!sliceImage)
    {
        return std::unexpected(false);
    }

    //
    // Apply post-processing to the rendered image
    //
    ApplyPostProcessing(*sliceImage, params);

    return sliceImage;
}

std::expected<ImageView, bool> ImageView::From(const NFITS::ImageSlice& slice, const ImageRenderParams& params, const NFITS::ImageData* pImageData)
{
    //
    // Sanity test validation
    //
    const auto& metadata = pImageData->GetMetadata();

    if (metadata.naxisns.size() < 2)
    {
        std::cerr << "ImageView::From: Image data must have at least 2 axes" << std::endl;
        return std::unexpected(false);
    }

    if (pImageData->GetPhysicalValues().empty())
    {
        std::cerr << "ImageView::From: Image data is empty" << std::endl;
        return std::unexpected(false);
    }

    //
    // Render the image data
    //
    const auto qImage = RenderImageData(slice, params, pImageData);
    if (!qImage)
    {
        return std::unexpected(false);
    }

    return ImageView(*qImage);
}

ImageView::ImageView(QImage image)
    : m_image(std::move(image))
{

}

}
