/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_UTIL_ENDIANNESS_H
#define NFITS_SRC_UTIL_ENDIANNESS_H

#include <algorithm>
#include <array>

namespace NFITS
{
    /**
     * Take an object of tightly packed bytes and swaps the order/endianness of
     * those bytes in place, if the target machine is not big endian.
     */
    template <typename T>
    inline constexpr void FixEndiannessPacked(T& value) noexcept
    {
        if constexpr (std::endian::native != std::endian::big)
        {
            auto valueBytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
            std::ranges::reverse(valueBytes);
            value = std::bit_cast<T>(valueBytes);
        }
    }
}

#endif //NFITS_SRC_UTIL_ENDIANNESS_H
