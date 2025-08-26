/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_UTIL_IMAGEUTIL_H
#define NFITS_INCLUDE_NFITS_UTIL_IMAGEUTIL_H

#include <NFITS/SharedLib.h>
#include <NFITS/Data/ImageData.h>

#include <utility>

namespace NFITS
{
    /**
     * Calculates a min/max range of physical values which match the percentile passed in. E.g. for 95% percentile,
     * pass in n=0.95, and it will return the physical range which cuts off the bottom and top 2.5% of the range.
     */
    NFITS_PUBLIC std::pair<double, double> CalculatePercentileRange(const PhysicalStats& physicalStats, float n);
}

#endif //NFITS_INCLUDE_NFITS_UTIL_IMAGEUTIL_H
