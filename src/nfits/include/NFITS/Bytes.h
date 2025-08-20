/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_BYTES_H
#define NFITS_INCLUDE_NFITS_BYTES_H

#include <cstdint>

namespace NFITS
{
    /**
     * Wrapper type that represents a byte size
     */
    struct ByteSize
    {
        constexpr explicit ByteSize(std::uintmax_t v) : value(v) {}

        auto operator<=>(const ByteSize&) const = default;

        std::uintmax_t value;
    };

    constexpr ByteSize operator+(ByteSize lhs, ByteSize rhs)    { return ByteSize(lhs.value + rhs.value); }
    constexpr ByteSize operator+(ByteSize lhs, uintmax_t rhs)   { return ByteSize(lhs.value + rhs); }
    constexpr ByteSize operator+(uintmax_t lhs, ByteSize rhs)   { return ByteSize(lhs + rhs.value); }

    constexpr ByteSize operator-(ByteSize lhs, ByteSize rhs)    { return ByteSize(lhs.value - rhs.value); }
    constexpr ByteSize operator-(ByteSize lhs, uintmax_t rhs)   { return ByteSize(lhs.value -+ rhs); }
    constexpr ByteSize operator-(uintmax_t lhs, ByteSize rhs)   { return ByteSize(lhs - rhs.value); }

    constexpr ByteSize operator*(ByteSize lhs, ByteSize rhs)    { return ByteSize(lhs.value * rhs.value); }
    constexpr ByteSize operator*(ByteSize lhs, uintmax_t rhs)   { return ByteSize(lhs.value * rhs); }
    constexpr ByteSize operator*(uintmax_t lhs, ByteSize rhs)   { return ByteSize(lhs * rhs.value); }

    constexpr ByteSize operator/(ByteSize lhs, ByteSize rhs)    { return ByteSize(lhs.value / rhs.value); }
    constexpr ByteSize operator/(ByteSize lhs, uintmax_t rhs)   { return ByteSize(lhs.value / rhs); }
    constexpr ByteSize operator/(uintmax_t lhs, ByteSize rhs)   { return ByteSize(lhs / rhs.value); }

    constexpr ByteSize operator%(ByteSize lhs, ByteSize rhs)    { return ByteSize(lhs.value % rhs.value); }
    constexpr ByteSize operator%(ByteSize lhs, uintmax_t rhs)   { return ByteSize(lhs.value % rhs); }
    constexpr ByteSize operator%(uintmax_t lhs, ByteSize rhs)   { return ByteSize(lhs % rhs.value); }

    constexpr bool operator==(uintmax_t lhs, ByteSize rhs)      { return lhs == rhs.value; }
    constexpr bool operator==(ByteSize lhs, uintmax_t rhs)      { return lhs.value == rhs; }

    /**
    * Wrapper type that represents an offset in bytes
    */
    struct ByteOffset
    {
        constexpr explicit ByteOffset(std::uintmax_t v) : value(v) {}
        constexpr explicit ByteOffset(const ByteSize& v) : value(v.value) {}

        auto operator<=>(const ByteOffset&) const = default;

        std::uintmax_t value;
    };

    constexpr ByteOffset operator+(ByteOffset lhs, ByteOffset rhs)  { return ByteOffset(lhs.value + rhs.value); }
    constexpr ByteOffset operator+(ByteOffset lhs, uintmax_t rhs)   { return ByteOffset(lhs.value + rhs); }
    constexpr ByteOffset operator+(uintmax_t lhs, ByteOffset rhs)   { return ByteOffset(lhs + rhs.value); }

    constexpr ByteOffset operator-(ByteOffset lhs, ByteOffset rhs)  { return ByteOffset(lhs.value - rhs.value); }
    constexpr ByteOffset operator-(ByteOffset lhs, uintmax_t rhs)   { return ByteOffset(lhs.value - rhs); }
    constexpr ByteOffset operator-(uintmax_t lhs, ByteOffset rhs)   { return ByteOffset(lhs - rhs.value); }

    constexpr ByteOffset operator*(ByteOffset lhs, ByteOffset rhs)  { return ByteOffset(lhs.value * rhs.value); }
    constexpr ByteOffset operator*(ByteOffset lhs, uintmax_t rhs)   { return ByteOffset(lhs.value * rhs); }
    constexpr ByteOffset operator*(uintmax_t lhs, ByteOffset rhs)   { return ByteOffset(lhs * rhs.value); }

    constexpr ByteOffset operator/(ByteOffset lhs, ByteOffset rhs)  { return ByteOffset(lhs.value / rhs.value); }
    constexpr ByteOffset operator/(ByteOffset lhs, uintmax_t rhs)   { return ByteOffset(lhs.value / rhs); }
    constexpr ByteOffset operator/(uintmax_t lhs, ByteOffset rhs)   { return ByteOffset(lhs / rhs.value); }

    constexpr ByteOffset operator%(ByteOffset lhs, ByteOffset rhs)  { return ByteOffset(lhs.value % rhs.value); }
    constexpr ByteOffset operator%(ByteOffset lhs, uintmax_t rhs)   { return ByteOffset(lhs.value % rhs); }
    constexpr ByteOffset operator%(uintmax_t lhs, ByteOffset rhs)   { return ByteOffset(lhs % rhs.value); }

    constexpr bool operator==(uintmax_t lhs, ByteOffset rhs)      { return lhs == rhs.value; }
    constexpr bool operator==(ByteOffset lhs, uintmax_t rhs)      { return lhs.value == rhs; }
}

#endif //NFITS_INCLUDE_NFITS_BYTES_H
