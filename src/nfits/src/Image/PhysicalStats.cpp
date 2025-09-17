/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Image/PhysicalStats.h>

#include <numeric>
#include <cmath>

namespace NFITS
{

static constexpr std::size_t HISTOGRAM_NUM_BINS = 100;

void CalculateMinMax(PhysicalStats& physicalStats, const std::vector<std::span<const double>>& values)
{
    physicalStats.minMax = {
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::lowest()
    };

    for (const auto& span : values)
    {
        for (const auto& value : span)
        {
            // Skip over nan/infinity values
            if (!std::isfinite(value)) { continue; }

            physicalStats.minMax.first = std::min(physicalStats.minMax.first, value);
            physicalStats.minMax.second = std::max(physicalStats.minMax.second, value);
        }
    }
}

void CalculateHistogram(PhysicalStats& physicalStats, const std::vector<std::span<const double>>& values)
{
    const auto rangeMin = physicalStats.minMax.first;
    const auto rangeMax = physicalStats.minMax.second;
    const auto rangeSpan = rangeMax - rangeMin;

    physicalStats.histogram = std::vector<std::size_t>(HISTOGRAM_NUM_BINS, 0);

    for (const auto& span : values)
    {
        for (const auto& value : span)
        {
            // Skip over nan/infinity values
            if (!std::isfinite(value)) { continue; }

            const auto binIndex = static_cast<std::size_t>(((value - rangeMin) / rangeSpan) * (double) (HISTOGRAM_NUM_BINS - 1));

            physicalStats.histogram[binIndex]++;
        }
    }
}

void CalculateHistogramCumulative(PhysicalStats& physicalStats)
{
    physicalStats.histogramCumulative = std::vector<std::size_t>(HISTOGRAM_NUM_BINS, 0);
    physicalStats.histogramCumulative.at(0) = physicalStats.histogram.at(0);

    for (std::size_t binIndex = 1; binIndex < HISTOGRAM_NUM_BINS; ++binIndex)
    {
        physicalStats.histogramCumulative.at(binIndex) = physicalStats.histogramCumulative.at(binIndex - 1) + physicalStats.histogram.at(binIndex);
    }
}

PhysicalStats CompilePhysicalStats(const std::vector<std::span<const double>>& values)
{
    PhysicalStats physicalStats{};

    CalculateMinMax(physicalStats, values);
    CalculateHistogram(physicalStats, values);
    CalculateHistogramCumulative(physicalStats);

    return physicalStats;
}

}
