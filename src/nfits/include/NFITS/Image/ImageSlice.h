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

#include "../WCS/WCSParams.h"

#include <span>
#include <cstdint>
#include <vector>
#include <expected>

namespace NFITS
{
    /**
     * A key which identifies a specific slice within an ImageSliceSpan
     */
    struct ImageSliceKey
    {
        auto operator<=>(const ImageSliceKey&) const = default;

        /**
         * Contains values for each axis in an ImageSliceSpan, ignoring the
         * first two base (width/height) axes.
         */
        std::vector<int64_t> axesValues;
    };

    /**
     * Defines all dimensions for a collection of image slices, including base
     * (width/height) data dimensions. Ordered by axis. Equivalent to naxisn
     * values.
     */
    struct ImageSliceSpan
    {
        auto operator<=>(const ImageSliceSpan&) const = default;

        std::vector<int64_t> axes;
    };

    /**
     * Contains the data relevant to a specific 2D slice of a N-dimensional image.
     *
     * Non-owning; only references the physical values from the owning ImageData.
     */
    struct ImageSlice
    {
        uint64_t width{0};
        uint64_t height{0};
        PhysicalStats physicalStats{};              // Physical stats compiled from the specific image slice
        PhysicalStats cubePhysicalStats{};          // Physical stats compiled from the slice cube the slice is contained within
        std::span<const double> physicalValues;     // Physical values for the image slice
        std::optional<std::string> physicalUnit;    // Optional string describing the physical values unit
        std::optional<WCSParams> wcsParams;         // Optional parameters for WCS transformation
    };

    /**
     * @return The total number of slices that a slice span encompasses
     */
    [[nodiscard]] NFITS_PUBLIC uint64_t GetNumSlicesInSpan(const ImageSliceSpan& span);

    /**
     * @return A key which (fully) identifies the first slice in the span
     */
    [[nodiscard]] NFITS_PUBLIC ImageSliceKey GetDefaultSliceKey(const ImageSliceSpan& span);

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
