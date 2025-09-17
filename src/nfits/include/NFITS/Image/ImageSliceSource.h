/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_IMAGESLICESOURCE_H
#define NFITS_INCLUDE_NFITS_IMAGE_IMAGESLICESOURCE_H

#include "ImageSlice.h"

namespace NFITS
{
    class ImageSliceSource
    {
        public:

            virtual ~ImageSliceSource() = default;

            [[nodiscard]] virtual ImageSliceSpan GetImageSliceSpan() const = 0;
            [[nodiscard]] virtual std::optional<ImageSlice> GetImageSlice(const ImageSliceKey& sliceKey) const = 0;
    };
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_IMAGESLICESOURCE_H
