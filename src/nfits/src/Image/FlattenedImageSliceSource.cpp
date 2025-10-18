/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Image/FlattenedImageSliceSource.h>

#include <algorithm>
#include <numeric>
#include <cassert>

namespace NFITS
{

std::expected<std::unique_ptr<FlattenedImageSliceSource>, Error>
FlattenedImageSliceSource::Create(std::vector<std::unique_ptr<ImageSliceSource>> sources)
{
    if (sources.empty())
    {
        return std::unexpected(Error::Msg("Must provide at least one source"));
    }

    std::optional<int64_t> width;
    std::optional<int64_t> height;
    uintmax_t globalNumSlices = 0;

    std::vector<std::span<const double>> globalSliceSpans;

    for (const auto& source : sources)
    {
        // Source must have at least 2 dimensions
        const auto localSpan = source->GetImageSliceSpan();
        if (localSpan.axes.size() < 2)
        {
            return std::unexpected(Error::Msg("Slice sources must be at least two dimensional"));
        }

        // Source width/height must match all other source's width/height
        const auto localWidth = localSpan.axes.at(0);
        const auto localHeight = localSpan.axes.at(1);

        if ((width && (*width != localWidth)) || (height && (*height != localHeight)))
        {
            return std::unexpected(Error::Msg("Slice sources must have matching base dimensions"));
        }

        width = localWidth;
        height = localHeight;

        // Record all of source's local slice spans
        const auto localSourceNumSlices = GetNumSlicesInSpan(localSpan);
        globalNumSlices += localSourceNumSlices;

        for (uintmax_t localSliceIndex = 0; localSliceIndex < localSourceNumSlices; ++localSliceIndex)
        {
            // Slice index -> Slice key
            const auto localSliceKey = SliceLinearIndexToKey(localSpan, localSliceIndex);
            if (!localSliceKey)
            {
                return std::unexpected(Error::Msg("Out of bounds local slice key"));
            }

            const auto localSlice = source->GetImageSlice(*localSliceKey);
            if (!localSlice)
            {
                return std::unexpected(Error::Msg("Out of bounds local slice"));
            }

            globalSliceSpans.push_back(localSlice->physicalValues);
        }
    }

    assert(globalNumSlices <= std::numeric_limits<int64_t>::max());

    const auto globalSpan = ImageSliceSpan{
        .axes = {*width, *height, static_cast<int64_t>(globalNumSlices)}
    };

    const auto globalPhysicalStats = CompilePhysicalStats(globalSliceSpans);

    return std::make_unique<FlattenedImageSliceSource>(
        Tag{},
        std::move(sources),
        globalSpan,
        globalPhysicalStats
    );
}

FlattenedImageSliceSource::FlattenedImageSliceSource(Tag,
                                                     std::vector<std::unique_ptr<ImageSliceSource>> sources,
                                                     ImageSliceSpan globalSpan,
                                                     PhysicalStats globalPhysicalStats)
    : m_sources(std::move(sources))
    , m_globalSpan(std::move(globalSpan))
    , m_globalPhysicalStats(std::move(globalPhysicalStats))
{

}

ImageSliceSpan FlattenedImageSliceSource::GetImageSliceSpan() const
{
    return m_globalSpan;
}

std::optional<std::pair<ImageSliceKey, ImageSliceSource*>> FlattenedImageSliceSource::GetLocalSource(const ImageSliceKey& sliceKey) const
{
    // Convert the global slice key to a global index
    const auto globalIndex = SliceKeyToLinearIndex(m_globalSpan, sliceKey);
    if (!globalIndex)
    {
        return std::nullopt;
    }

    // Iterate through our sources, accumulating slice counts, until we've found a source containing
    // the global slice index
    auto indexRemaining = *globalIndex;

    for (const auto& source : m_sources)
    {
        const auto localSourceSpan = source->GetImageSliceSpan();
        const auto localSourceNumSlices = GetNumSlicesInSpan(localSourceSpan);

        if (indexRemaining < localSourceNumSlices)
        {
            const auto localKey = SliceLinearIndexToKey(localSourceSpan, indexRemaining);
            if (!localKey)
            {
                return std::nullopt;
            }

            return std::make_pair(*localKey, source.get());
        }

        indexRemaining -= localSourceNumSlices;
    }

    return std::nullopt;
}

std::optional<ImageSliceKey> FlattenedImageSliceSource::GetLocalKey(const ImageSliceKey& sliceKey)
{
    // Convert global slice key to (local slice key, local source)
    const auto localSource = GetLocalSource(sliceKey);
    if (!localSource)
    {
        return std::nullopt;
    }

    return localSource->first;
}

std::optional<ImageSlice> FlattenedImageSliceSource::GetImageSlice(const ImageSliceKey& sliceKey) const
{
    // Convert global slice key to (local slice key, local source)
    const auto localSource = GetLocalSource(sliceKey);
    if (!localSource)
    {
        return std::nullopt;
    }

    // Fetch the local slice
    auto slice = localSource->second->GetImageSlice(localSource->first);
    if (!slice)
    {
        return std::nullopt;
    }

    // Overwrite the local slice's cube stats with this collection's global physical stats
    slice->cubePhysicalStats = m_globalPhysicalStats;

    return slice;
}

}
