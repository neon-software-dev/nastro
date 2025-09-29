/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Image/ImageSlice.h>

#include <cassert>
#include <numeric>

namespace NFITS
{

uint64_t GetNumSlicesInSpan(const ImageSliceSpan& span)
{
    if (span.empty())       { return 0; }
    if (span.size() <= 2)   { return 1; }

    return std::accumulate(span.cbegin() + 2, span.cend(), 1U, std::multiplies<>());
}

std::expected<uintmax_t, Error> SliceKeyToLinearIndex(const ImageSliceSpan& span, const ImageSliceKey& key)
{
    if (span.size() > 999)
    {
        return std::unexpected(Error::Msg("SliceKeyToLinearIndex: Out of bounds axis count"));
    }

    uintmax_t currentMultiple = 1;
    uintmax_t index = 0;

    for (std::size_t x = 2; x < span.size(); ++x)
    {
        const auto it = key.find(static_cast<unsigned int>(x) + 1);
        const auto axisValue = it != key.cend() ? it->second : 0;

        index += (currentMultiple * axisValue);
        currentMultiple *= span.at(x);
    }

    return index;
}

std::expected<ImageSliceKey, Error> SliceLinearIndexToKey(const ImageSliceSpan& span, const uintmax_t& index)
{
    if (index >= GetNumSlicesInSpan(span))
    {
        return std::unexpected(Error::Msg("SliceLinearIndexToKey: Out of bounds index"));
    }

    // If 2D or less, only 1 slice ever exists, return a default key
    if (span.size() <= 2)
    {
        return ImageSliceKey{};
    }

    ImageSliceKey key{};

    uintmax_t working = index;

    for (std::size_t x = span.size() - 1; x >= 2; --x)
    {
        const uintmax_t higherAxesMultiple =
            std::accumulate(span.cbegin() + 2,
                            span.cbegin() + static_cast<ImageSliceSpan::difference_type>(x),
                            1U,
                            std::multiplies<>());

        const auto quotient = working / higherAxesMultiple;

        key.insert({x + 1, quotient});

        working -= quotient * higherAxesMultiple;
    }

    return key;
}

}
