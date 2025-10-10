/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_IMAGECOMMON_H
#define NFITS_INCLUDE_NFITS_IMAGE_IMAGECOMMON_H

#include <unordered_map>
#include <cstdint>
#include <optional>
#include <vector>
#include <utility>
#include <array>

namespace NFITS
{
    /**
     * Transfer function to map normalized physical value to display value
     */
    enum class TransferFunction
    {
        Linear,
        Log,
        Sqrt,
        Square,
        Asinh
    };

    /**
     * Color maps which can be applied to map display values to RGB colors
     */
    enum class ColorMap
    {
        // Custom color maps
        Fire, Ocean, Ice,

        // Linear
        CET_L01, CET_L02, CET_L03, CET_L04, CET_L05, CET_L06, CET_L07, CET_L08, CET_L09, CET_L10,
        CET_L11, CET_L12, CET_L13, CET_L14, CET_L15, CET_L16, CET_L17, CET_L18, CET_L19, CET_L20,

        // Diverging
        CET_D01, CET_D01A, CET_D02, CET_D03, CET_D04, CET_D06, CET_D07, CET_D08, CET_D09, CET_D10,
        CET_D13, CET_R3,

        // Rainbow
        CET_R1, CET_R2, CET_R4
    };

    enum class ScalingMode
    {
        /**
         * Transfer function will use the physical value range from each particular image/slice
         */
        PerImage,

        /**
         * Transfer function will use the physical value range from the entire image cube a particular
         * image/slice is part of. (Results in the same thing as PerImage if there's no image cube)
         */
        PerCube
    };

    enum class ScalingRange
    {
        Full,
        p99,
        p95,
        Custom
    };

    /**
     * Configuration that controls how image data is rendered into a displayable image
     */
    struct ImageRenderParams
    {
        /** Transfer function to apply */
        TransferFunction transferFunction{TransferFunction::Linear};

        /** Scaling mode for image transfer function to use */
        ScalingMode scalingMode{ScalingMode::PerImage};

        /** Scaling range for image transfer function to use */
        ScalingRange scalingRange{ScalingRange::p99};

        /** Scaling range min/max used for ScalingRange::Custom */
        std::optional<double> customScalingRangeMin;
        std::optional<double> customScalingRangeMax;

        /** Log base applied when using TransferFunction::Log */
        double logTransferBase = 100.0;

        /** Scale factor applied when using TransferFunction::Asinh */
        double asinhTransferScale = 15.0;

        /** Color map to apply to generate pixel colors */
        ColorMap colorMap{ColorMap::CET_L01};

        /** Whether to invert the final image pixel colors */
        bool invertColors{false};

        /** Color to apply to blank pixels, default: black */
        std::array<unsigned char, 3> blankColor{0};
    };
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_IMAGECOMMON_H
