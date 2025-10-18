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

        sliceSpan.axes.push_back(naxisn);
    }

    return sliceSpan;
}

}
