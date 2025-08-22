/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_IMAGE_IMAGEVIEW_H
#define SRC_IMAGE_IMAGEVIEW_H

#include <QImage>

#include <expected>
#include <unordered_map>

namespace Nastro
{
    class ImageData;

    enum class TransferFunction
    {
        Linear,
        Log,
        Sqrt,
        Square
    };

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

    /**
     * Helper class which converts image data from a FITS file into a displayable QImage
     */
    class ImageView
    {
        public:

            /**
             * Configuration that controls which part/slice of a n-dimensional image is viewed
             */
            struct Selection
            {
                // Values for slice selection for axis 3+. If not set for an axis, slice
                // selection logic will default to an axis value of 0.
                //
                // axisn [3..naxis] -> axis value [0..naxisn)
                std::unordered_map<unsigned int, int64_t> axisSelection;
            };

            /**
             * Configuration that controls how image data is turned into a displayable image
             */
            struct Params
            {
                // Physical value -> pixel transfer function
                TransferFunction transferFunction{TransferFunction::Linear};

                // Log base applied when using TransferFunction::Log
                double logTransferBase = 100.0;

                // Color map to apply to determine pixel colors
                ColorMap colorMap{ColorMap::CET_L01};

                // Whether to invert the final image pixel colors
                bool invertColors{false};
            };

        public:

            [[nodiscard]] static std::expected<ImageView, bool> From(const Selection& selection, const Params& params, const ImageData* pImageData);

            [[nodiscard]] const QImage& GetImage() const noexcept { return m_image; }

        public:

            ImageView() = default;

            explicit ImageView(QImage image);

        private:

            QImage m_image{};
    };
}

#endif //SRC_IMAGE_IMAGEVIEW_H
