/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_IMAGESLICE_H
#define NFITS_INCLUDE_NFITS_IMAGE_IMAGESLICE_H

#include "ImageCommon.h"
#include "PhysicalStats.h"

#include "../Error.h"
#include "../SharedLib.h"

#include <span>
#include <cstdint>
#include <vector>
#include <expected>

namespace NFITS
{
    /**
     * Axis values which identify a specific 2D slice of a N-dimensional image.
     *
     * If a value is not provided for an axis, a value of 0 is assumed.
     *
     * Keys specify axis and should be 1-based [3..naxis]
     * Values specify axis value and should be 0-based [0..naxisn)
     */
    using ImageSliceKey = std::unordered_map<unsigned int, uint64_t>;

    /**
     * Defines all dimensions for a collection of image slices. Ordered by axis. Equivalent to
     * naxisn values for each axis in an ImageData.
     */
    using ImageSliceSpan = std::vector<uint64_t>;

    /**
     * Contains the data relevant to a specific 2D slice of a N-dimensional image.
     *
     * Non-owning; only references the physical values from the owning ImageData.
     */
    struct ImageSlice
    {
        uint64_t width{0};
        uint64_t height{0};
        PhysicalStats physicalStats{};          // Physical stats compiled from the specific image slice
        PhysicalStats cubePhysicalStats{};      // Physical stats compiled from the slice cube the slice is contained within
        std::span<const double> physicalValues; // Physical values for the image slice
    };

    /**
     * @return The total number of slices that a slice span encompasses
     */
    [[nodiscard]] NFITS_PUBLIC uint64_t GetNumSlicesInSpan(const ImageSliceSpan& span);

    /**
     * Given a slice span and key, returns the (zero-based) linear index of the slice within the span.
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<uintmax_t, Error> SliceKeyToLinearIndex(const ImageSliceSpan& span,
                                                                                     const ImageSliceKey& key);

    /**
     * Performs the reverse of SliceKeyToLinearIndex - transforms a linear index from a slice span into the
     * slice key that identifies that index.
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<ImageSliceKey, Error> SliceLinearIndexToKey(const ImageSliceSpan& span,
                                                                                         const uintmax_t& index);
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_IMAGESLICE_H
