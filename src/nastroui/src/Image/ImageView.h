/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_IMAGE_IMAGEVIEW_H
#define SRC_IMAGE_IMAGEVIEW_H

#include "ImageCommon.h"

#include <NFITS/Data/ImageData.h>

#include <QImage>

#include <expected>

namespace Nastro
{
    /**
     * Helper class which converts image data from a FITS file into a displayable QImage
     */
    class ImageView
    {
        public:

            [[nodiscard]] static std::expected<ImageView, bool> From(const NFITS::ImageSlice& slice, const ImageRenderParams& params, const NFITS::ImageData* pImageData);

            [[nodiscard]] const QImage& GetImage() const noexcept { return m_image; }

        public:

            ImageView() = default;

            explicit ImageView(QImage image);

        private:

            QImage m_image{};
    };
}

#endif //SRC_IMAGE_IMAGEVIEW_H
