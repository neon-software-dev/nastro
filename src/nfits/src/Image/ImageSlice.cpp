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
    if (span.axes.empty()) { return 0; }
    if (span.axes.size() == 1) { return 0; }
    if (span.axes.size() == 2) { return 1; }

    return std::accumulate(span.axes.cbegin() + 2, span.axes.cend(), uint64_t{1U}, std::multiplies<>());
}

ImageSliceKey GetDefaultSliceKey(const ImageSliceSpan& span)
{
    ImageSliceKey sliceKey{};

    if (span.axes.size() > 2)
    {
        sliceKey.axesValues.resize(span.axes.size() - 2, 0);
    }

    return sliceKey;
}

std::expected<uintmax_t, Error> SliceKeyToLinearIndex(const ImageSliceSpan& span, const ImageSliceKey& key)
{
    if ((span.axes.size() < 2) || (span.axes.size() > 999))
    {
        return std::unexpected(Error::Msg("SliceKeyToLinearIndex: Out of bounds axis count"));
    }

    if (key.axesValues.size() != (span.axes.size() - 2))
    {
        return std::unexpected(Error::Msg("SliceKeyToLinearIndex: Invalid ImageSliceKey for provided ImageSliceSpan"));
    }

    uintmax_t currentMultiple = 1;
    uintmax_t index = 0;

    for (std::size_t x = 2; x < span.axes.size(); ++x)
    {
        const auto axisValue = key.axesValues.at(x - 2);

        index += (currentMultiple * static_cast<uintmax_t>(axisValue));
        currentMultiple *= static_cast<uintmax_t>(span.axes.at(x));
    }

    return index;
}

std::expected<ImageSliceKey, Error> SliceLinearIndexToKey(const ImageSliceSpan& span, const uintmax_t& index)
{
    if (index >= GetNumSlicesInSpan(span))
    {
        return std::unexpected(Error::Msg("SliceLinearIndexToKey: Out of bounds index"));
    }

    // Return a default/empty key if the span is 2D and has no slices
    if (span.axes.size() == 2)
    {
        return ImageSliceKey{};
    }

    ImageSliceKey key{};
    key.axesValues.resize(span.axes.size() - 2, 0);

    uintmax_t working = index;

    for (std::size_t x = span.axes.size() - 1; x >= 2; --x)
    {
        const auto higherAxesMultiple =
            std::accumulate(span.axes.cbegin() + 2U,
                            span.axes.cbegin() + static_cast<std::vector<int64_t>::difference_type>(x),
                            uint64_t{1},
                            std::multiplies<>());

        const auto quotient = working / higherAxesMultiple;

        key.axesValues.at(x - 2) = static_cast<int64_t>(quotient);

        working -= quotient * higherAxesMultiple;
    }

    return key;
}

}
