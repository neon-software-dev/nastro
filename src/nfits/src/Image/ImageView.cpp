/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Image/ImageView.h>

#include "../ColorMaps/colormap.h"
#include "../ColorMaps/cet_lut.h"

#include <NFITS/Util/ImageUtil.h>

#include <iostream>
#include <algorithm>
#include <cassert>

namespace NFITS
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

void OutputPixel(unsigned char* pScanline, uint64_t x, std::span<const unsigned char> pixelComponents)
{
    const auto scanlineOffset = x * pixelComponents.size();

    for (unsigned int component = 0; component < pixelComponents.size(); ++component)
    {
        pScanline[scanlineOffset + component] = pixelComponents[component];
    }
}

std::expected<ImageRender, bool> PhysicalValuesToImage(const ImageSlice& imageSlice, const ImageRenderParams& params)
{
    //
    // Fetch the physical stats of the data to be rendered
    //
    std::optional<NFITS::PhysicalStats> physicalStats;

    switch (params.scalingMode)
    {
        case ScalingMode::PerImage: { physicalStats = imageSlice.physicalStats; } break;
        case ScalingMode::PerCube:  { physicalStats = imageSlice.cubePhysicalStats; } break;
    }

    if (!physicalStats)
    {
        std::cerr << "PhysicalValuesToImage: Failed to determine slice physical stats" << std::endl;
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
    // Fill an ImageRender with interpreted image data
    //
    auto imageRender = ImageRender(ImageRender::Format::RGB888, imageSlice.width, imageSlice.height);

    for (uint64_t y = 0; y < imageSlice.height; ++y)
    {
        // Note that unofficial FITS standard is for images to be stored bottom to top, so y=0
        // corresponds to the bottom scanline, visually, of the image
        unsigned char* pScanline = imageRender.GetScanLineBytesStart(y);

        for (uint64_t x = 0; x < imageSlice.width; ++x)
        {
            const auto physicalValueIndex = x + (y * imageSlice.width);

            auto physicalValue = imageSlice.physicalValues[physicalValueIndex];

            // If the physical value is nan, signifying a blank value, then output the
            // blank color for the pixel and immediately continue to the next pixel
            if (std::isnan(physicalValue))
            {
                OutputPixel(pScanline, x, params.blankColor);
                continue;
            }

            // Otherwise, sanity check that physical value is within the expected range
            physicalValue = std::clamp(physicalValue, physicalValueMin, physicalValueMax);

            // Normalize the physical value
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
            // Output the final pixel color
            //
            OutputPixel(pScanline, x, rgbColors);
        }
    }

    return imageRender;
}

void ApplyPostProcessing(ImageRender& imageRender, const ImageRenderParams& params)
{
    for (std::size_t y = 0; y < imageRender.height; ++y)
    {
        const auto pScanLine = imageRender.GetScanLineBytesStart(y);

        for (std::size_t x = 0; x < imageRender.width; ++x)
        {
            if (params.invertColors)
            {
                for (std::size_t comp = 0; comp < imageRender.BytesPerPixel(); ++comp)
                {
                    const auto scanLineByteIndex = (x * imageRender.BytesPerPixel()) + comp;
                    pScanLine[scanLineByteIndex] = 255 - pScanLine[scanLineByteIndex];
                }
            }
        }
    }
}

std::expected<ImageRender, bool> RenderImageData(const ImageSlice& imageSlice, const ImageRenderParams& params)
{
    //
    // Process the slice's physical values into a rendered image
    //
    auto sliceImage = PhysicalValuesToImage(imageSlice, params);
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

std::expected<ImageView, bool> ImageView::Render(const ImageSlice& imageSlice, const ImageRenderParams& params)
{
    //
    // Render the image data
    //
    auto imageRender = RenderImageData(imageSlice, params);
    if (!imageRender)
    {
        return std::unexpected(false);
    }

    return ImageView(std::move(*imageRender));
}

ImageView::ImageView(ImageRender image)
    : m_imageRender(std::move(image))
{

}

}
