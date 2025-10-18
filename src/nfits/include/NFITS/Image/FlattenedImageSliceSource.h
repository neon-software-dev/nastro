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

        public:

            /**
             * Converts a (global) slice key for this source into the original, local, slice key
             * from the corresponding original ImageSliceSource.
             */
            [[nodiscard]] std::optional<ImageSliceKey> GetLocalKey(const ImageSliceKey& sliceKey);

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

            /**
             * @return (local slice key, local source) from a given global slice key (or std::nullopt if not exists)
             */
            [[nodiscard]] std::optional<std::pair<ImageSliceKey, ImageSliceSource*>> GetLocalSource(const ImageSliceKey& sliceKey) const;

        private:

            std::vector<std::unique_ptr<ImageSliceSource>> m_sources;
            ImageSliceSpan m_globalSpan;
            PhysicalStats m_globalPhysicalStats;
    };
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_FLATTENEDIMAGESLICESOURCE_H
