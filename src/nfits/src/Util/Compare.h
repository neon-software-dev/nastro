/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_UTIL_COMPARE_H
#define NFITS_SRC_UTIL_COMPARE_H

#include <numeric>
#include <complex>
#include <optional>

namespace NFITS
{
    template <typename T>
    requires std::floating_point<T>
    [[nodiscard]] static inline bool AreEqual(const T& a, const T& b)
    {
        return std::fabs(a - b) <= std::numeric_limits<double>::epsilon();
    }

    template <typename T>
    requires std::floating_point<T>
    [[nodiscard]] static inline bool AreEqualOpt(const std::optional<T>& a, const std::optional<T>& b)
    {
        if (!a && !b) { return true; }

        if (!a && b) { return false; }
        if (a && !b) { return false; }

        return AreEqual(*a, *b);
    }
}

#endif //NFITS_SRC_UTIL_COMPARE_H
