/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_UTIL_IMAGEUTILINTERNAL_H
#define NFITS_SRC_UTIL_IMAGEUTILINTERNAL_H

#include <NFITS/Error.h>
#include <NFITS/Image/ImageSlice.h>

#include <cstdint>
#include <expected>
#include <vector>

namespace NFITS
{
    /**
     * Transforms naxisns into an ImageSliceSpan.
     *
     * Effectively just transforms each naxisn to a positive integer.
     */
    [[nodiscard]] std::expected<ImageSliceSpan, Error> NaxisnsToSliceSpan(const std::vector<int64_t>& naxisns);
}

#endif //NFITS_SRC_UTIL_IMAGEUTILINTERNAL_H
