/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_FLATTENEDIMAGESLICESOURCE_H
#define NFITS_INCLUDE_NFITS_IMAGE_FLATTENEDIMAGESLICESOURCE_H

#include "ImageSliceSource.h"

#include "../SharedLib.h"

#include <memory>
#include <vector>

namespace NFITS
{
    /**
     * ImageSliceSource which composites a collection of any-dimensional slice sources into
     * a flattened, linear, 3D slice source.
     */
    class NFITS_PUBLIC FlattenedImageSliceSource : public ImageSliceSource
    {
        public:

            [[nodiscard]] static std::expected<std::unique_ptr<FlattenedImageSliceSource>, Error> Create(
                std::vector<std::unique_ptr<ImageSliceSource>> sources
            );

            //
            // ImageSliceSource
            //
            [[nodiscard]] ImageSliceSpan GetImageSliceSpan() const override;
            [[nodiscard]] std::optional<ImageSlice> GetImageSlice(const ImageSliceKey& sliceKey) const override;

        private:

            struct Tag{};

        public:

            FlattenedImageSliceSource(Tag,
                                      std::vector<std::unique_ptr<ImageSliceSource>> sources,
                                      ImageSliceSpan globalSpan,
                                      PhysicalStats globalPhysicalStats);

        private:

            std::vector<std::unique_ptr<ImageSliceSource>> m_sources;
            ImageSliceSpan m_globalSpan;
            PhysicalStats m_globalPhysicalStats;
    };
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_FLATTENEDIMAGESLICESOURCE_H
