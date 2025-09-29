/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageUtilInternal.h"

namespace NFITS
{

std::expected<ImageSliceSpan, Error> NaxisnsToSliceSpan(const std::vector<int64_t>& naxisns)
{
    ImageSliceSpan sliceSpan{};

    for (const auto& naxisn : naxisns)
    {
        if (naxisn < 0) { return std::unexpected(Error::Msg("Out of bounds naxisn value: {}", naxisn)); }

        sliceSpan.push_back(static_cast<uint64_t>(naxisn));
    }

    return sliceSpan;
}

}
