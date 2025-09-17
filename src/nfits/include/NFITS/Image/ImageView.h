/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_IMAGEVIEW_H
#define NFITS_INCLUDE_NFITS_IMAGE_IMAGEVIEW_H

#include "ImageRender.h"
#include "ImageCommon.h"

#include "../SharedLib.h"

#include "../Data/ImageData.h"

#include <expected>

namespace NFITS
{
    /**
     * Produces a displayable image render from physical image data, given a slice of
     * the image data to be viewed, and parameters for how to render the image.
     */
    class NFITS_PUBLIC ImageView
    {
        public:

            [[nodiscard]] static std::expected<ImageView, bool> From(const ImageSlice& imageSlice, const ImageRenderParams& params);

            [[nodiscard]] const ImageRender& GetImageRender() const noexcept { return m_imageRender; }

        public:

            ImageView() = default;

            explicit ImageView(ImageRender image);

        private:

            ImageRender m_imageRender;
    };
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_IMAGEVIEW_H
