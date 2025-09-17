/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_PHYSICALSTATS_H
#define NFITS_INCLUDE_NFITS_IMAGE_PHYSICALSTATS_H

#include <NFITS/SharedLib.h>

#include <vector>
#include <utility>
#include <span>

namespace NFITS
{
    /**
     * Contains calculated statistics about image physical values
     */
    struct PhysicalStats
    {
        /**
         * Min/Max of the physical values
         */
        std::pair<double, double> minMax;

        /**
         * Histogram of the physical values
         */
        std::vector<std::size_t> histogram;

        /**
         * Cumulative Histogram of the physical values
         */
        std::vector<std::size_t> histogramCumulative;
    };

    /**
     * Takes in image physical values and returns PhysicalStats calculated from those values
     */
    [[nodiscard]] NFITS_PUBLIC PhysicalStats CompilePhysicalStats(const std::vector<std::span<const double>>& values);
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_PHYSICALSTATS_H
