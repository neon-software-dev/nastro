/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageView.h"

#include "../Data/ImageData.h"
#include "../ColorMaps/colormap.h"
#include "../ColorMaps/cet_lut.h"

#include <iostream>
#include <limits>
#include <bit>
#include <cstddef>
#include <algorithm>

namespace Nastro
{

template <typename T>
inline constexpr void SwapEndiannessPacked(T& value) noexcept
{
    auto valueBytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(valueBytes);
    value = std::bit_cast<T>(valueBytes);
}

template <typename DataType>
double GetPhysicalValue(std::span<const DataType> data,
                        uintmax_t dataIndex,
                        const std::optional<double> bzero,
                        const std::optional<double> bscale)
{
    DataType dataValue = data[dataIndex];

    // FITS data is stored as big endian; swap endianness as needed for the current CPU
    if constexpr (std::endian::native != std::endian::big)
    {
        SwapEndiannessPacked(dataValue);
    }

    // Use bzero/bscale to calculate physical value, if available, or just return the data value if not
    if (bzero || bscale)
    {
        /**
         * [4.4.2.5]
         * physical value = BZERO + BSCALE × array value.
         */
        return *bzero + (*bscale * static_cast<double>(dataValue));
    }
    else
    {
        return (double)dataValue;
    }
}

std::pair<double, double> CalculatePhysicalMinMax(const std::vector<double>& physicalValues)
{
    auto physicalValueMin = std::numeric_limits<double>::max();
    auto physicalValueMax = std::numeric_limits<double>::lowest();

    for (const auto& physicalValue : physicalValues)
    {
        physicalValueMin = std::min(physicalValueMin, physicalValue);
        physicalValueMax = std::max(physicalValueMax, physicalValue);
    }

    return {physicalValueMin, physicalValueMax};
}

// For an image with more than 2 dimensions, calculates the data offset (not byte offset) of a particular
// 2D image slice, defined by selection's values
uintmax_t CalculateSelectionDataOffset(const ImageView::Selection& selection, const ImageParams& imageParams)
{
    uintmax_t dataOffset = 0;

    for (unsigned int n = 2; n < imageParams.naxisns.size(); ++n)
    {
        int64_t axisSelection = 0;

        const auto nValue = selection.axisSelection.find(n + 1);
        if (nValue != selection.axisSelection.cend())
        {
            axisSelection = nValue->second;
        }

        uintmax_t prevAxesDataSpan = 1;
        for (unsigned int p = 0; p < n; ++p)
        {
            prevAxesDataSpan *= static_cast<uintmax_t>(imageParams.naxisns.at(p));
        }

        dataOffset += prevAxesDataSpan * static_cast<uintmax_t>(axisSelection);
    }

    return dataOffset;
}

template <typename DataType>
std::expected<std::vector<double>, bool> ImageDataToPhysicalValues(const ImageView::Selection& selection, const ImageData* pImageData)
{
    const auto& imageParams = pImageData->GetParams();

    const auto imageWidth       = imageParams.naxisns.at(0);
    const auto imageHeight      = imageParams.naxisns.at(1);
    const auto sliceDataOffset  = CalculateSelectionDataOffset(selection, imageParams);
    const bool hasScaling       = imageParams.bZero || imageParams.bScale;

    if (hasScaling && !imageParams.bZero)
    {
        std::cerr << "DataToImageTyped: If bzero or bscaling are present, both must be" << std::endl;
        return std::unexpected(false);
    }
    if (hasScaling && !imageParams.bScale)
    {
        std::cerr << "DataToImageTyped: If bzero or bscaling are present, both must be" << std::endl;
        return std::unexpected(false);
    }

    // Interpret the image data as an array of the specific data type values
    const auto typedImageData = std::span<const DataType>(
        reinterpret_cast<const DataType*>(pImageData->GetData().data()),
        pImageData->GetData().size_bytes() / sizeof(DataType)
    );

    std::vector<double> physicalValues;
    physicalValues.reserve((std::size_t)imageWidth * (std::size_t)imageHeight);

    for (int64_t y = 0; y < imageHeight; ++y)
    {
        for (int64_t x = 0; x < imageWidth; ++x)
        {
            const auto dataIndex = static_cast<uintmax_t>(x + (y * imageWidth)) + sliceDataOffset;
            const auto physicalValue = GetPhysicalValue(typedImageData, dataIndex, imageParams.bZero, imageParams.bScale);

            physicalValues.push_back(physicalValue);
        }
    }

    return physicalValues;
}

QImage PhysicalValuesToQImage(const ImageView::Params& params, const ImageData* pImageData, const std::vector<double>& physicalValues)
{
    const auto& imageParams = pImageData->GetParams();

    const auto imageWidth = imageParams.naxisns.at(0);
    const auto imageHeight = imageParams.naxisns.at(1);

    auto physicalValueMin = imageParams.dataMin;
    auto physicalValueMax = imageParams.dataMax;

    // If datamin/datamax weren't provided, calculate them ourselves from the data
    if (!physicalValueMin || !physicalValueMax)
    {
        const auto physicalMinMax = CalculatePhysicalMinMax(physicalValues);
        physicalValueMin = physicalMinMax.first;
        physicalValueMax = physicalMinMax.second;
    }

    const double physicalValueRange = *physicalValueMax - *physicalValueMin;

    // Create a QImage and fill it with interpreted image data
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

            const double physicalValue = physicalValues.at(physicalValueIndex);

            double norm = (physicalValue - *physicalValueMin) / physicalValueRange;

            // Force clamp norm to be within [0.0..1.0]. There's edge cases where a file provides DATAMIN/DATAMAX that
            // are specified in a lower level of precision, and when we do the math to calculate physical value, for the
            // values that are at min/max, we can end up with some extra precision randomness in the calculated physical value
            // where when we calculate the norm above, it causes norm to be extremely slightly below/above 0.0..1.0
            norm = std::clamp(norm, 0.0, 1.0);

            //
            // Apply transfer function to map normalized value to display value
            //
            double displayValuePercent = 0.0;

            switch (params.transferFunction)
            {
                case TransferFunction::Linear:  displayValuePercent = norm; break;
                case TransferFunction::Log:     displayValuePercent = std::log1p(norm * (params.logTransferBase - 1.0)) / std::log(params.logTransferBase); break;
                case TransferFunction::Sqrt:    displayValuePercent = std::pow(norm, 1.0 / 2.0); break;
                case TransferFunction::Square:  displayValuePercent = std::pow(norm, 1.0 / 0.5); break;
            }

            //
            // Apply color map function to map display value to RGB color
            //
            std::optional<std::array<double, 3>> colorPercentages; // Filled in with color percentages below if a pre-defined colormap is configured
            std::optional<const std::array<uint8_t, 768>*> cet_lut; // Filled in with a LUT below if a specific CET is configured

            switch (params.colorMap)
            {
                case ColorMap::Gray:    { colorPercentages = {0.0}; colormap::ramp::gray(displayValuePercent, colorPercentages->data()); } break;
                case ColorMap::Fire:    { colorPercentages = {0.0}; colormap::ramp::fire(displayValuePercent, colorPercentages->data()); } break;
                case ColorMap::Ocean:   { colorPercentages = {0.0}; colormap::ramp::ocean(displayValuePercent, colorPercentages->data()); } break;
                case ColorMap::Ice:     { colorPercentages = {0.0}; colormap::ramp::ice(displayValuePercent, colorPercentages->data()); } break;

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
                const std::size_t offset = static_cast<unsigned char>(displayValuePercent * 255.0) * 3;
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

std::expected<QImage, bool> ImageDataToQImage(const ImageView::Selection& selection, const ImageView::Params& params, const ImageData* pImageData)
{
    std::expected<std::vector<double>, bool> physicalValues;

    //
    // Convert image data values to physical values
    //
    switch (pImageData->GetParams().bitpix)
    {
        case 8: physicalValues      = ImageDataToPhysicalValues<uint8_t>(selection, pImageData); break;
        case 16: physicalValues     = ImageDataToPhysicalValues<int16_t>(selection, pImageData); break;
        case 32: physicalValues     = ImageDataToPhysicalValues<int32_t>(selection, pImageData); break;
        case -32: physicalValues    = ImageDataToPhysicalValues<float>(selection, pImageData); break;
        case -64: physicalValues    = ImageDataToPhysicalValues<double>(selection, pImageData); break;
        default: return std::unexpected(false);
    }
    if (!physicalValues)
    {
        return std::unexpected(false);
    }

    //
    // Convert physical values to a displayable QImage
    //
    auto qImage = PhysicalValuesToQImage(params, pImageData, *physicalValues);

    //
    // Apply pixel post-processing
    //
    for (int y = 0; y < qImage.height(); ++y)
    {
        auto pScanline = qImage.scanLine(y);

        for (int x = 0; x < qImage.width(); ++x)
        {
            if (params.invertColors)
            {
                for (unsigned int v = 0; v < 3; ++v)
                {
                    pScanline[(x * 3) + v] = 255 - pScanline[(x * 3) + v];
                }
            }
        }
    }

    return qImage;
}

std::expected<ImageView, bool> ImageView::From(const Selection& selection, const Params& params, const ImageData* pImageData)
{
    const auto& imageParams = pImageData->GetParams();

    //
    // Sanity test validation
    //
    if (imageParams.naxisns.size() < 2)
    {
        std::cerr << "ImageView::From: Image data must have at least 2 axes" << std::endl;
        return std::unexpected(false);
    }

    if (pImageData->GetData().empty())
    {
        std::cerr << "ImageView::From: Image data is empty" << std::endl;
        return std::unexpected(false);
    }

    //
    // Convert image data to a QImage
    //
    const auto qImage = ImageDataToQImage(selection, params, pImageData);
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
