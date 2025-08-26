/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Util/ImageUtil.h>

#include <ranges>
#include <cassert>

namespace NFITS
{

std::pair<double, double> CalculatePercentileRange(const PhysicalStats& physicalStats, float n)
{
    assert(physicalStats.histogramCumulative.size() > 2);

    // Percentage of data points to cut out from each side of the range (e.g. n=0.95, 0.025 from each end)
    const float cutOutPercent = (1.0f - n) / 2.0f;

    // Number of data points to cut out from each side of the range
    const auto numDataPoints = physicalStats.histogramCumulative.at(physicalStats.histogramCumulative.size() - 1);
    const auto cutOutCount = std::size_t(cutOutPercent * (double)(numDataPoints));

    // Walk forwards through the cumulative histogram until we've passed through a change of >= cutOutCount data points
    std::size_t countPassed = 0;
    std::size_t lowBin = 0;
    for (std::size_t x = 0; x < physicalStats.histogramCumulative.size(); ++x)
    {
        countPassed += physicalStats.histogramCumulative.at(x);
        if (countPassed >= cutOutCount)
        {
            lowBin = x;
            break;
        }
    }

    // Walk backwards through the cumulative histogram until we've passed through a change of >= cutOutCount data points
    countPassed = 0;
    std::size_t highBin = 0;
    std::size_t index = physicalStats.histogramCumulative.size() - 2;
    for (const auto& value : physicalStats.histogramCumulative | std::views::reverse)
    {
        countPassed += value - physicalStats.histogramCumulative.at(index);
        if (countPassed >= cutOutCount)
        {
            highBin = index + 1;
            break;
        }

        if (index == 0) break;

        index--;
    }

    // Round high-bin up to favor taking in the extra amount in the bin rather than losing it
    highBin = std::min(highBin + 1,  physicalStats.histogramCumulative.size() - 1);

    // Transform bin indices into bin percentages
    const double lowPercent = (double)lowBin / (double)physicalStats.histogram.size();
    const double highPercent = (double)highBin / (double)physicalStats.histogram.size();

    // Apply bin percentages to physical range
    const auto physicalValueRange = physicalStats.minMax.second - physicalStats.minMax.first;
    const auto min = (physicalValueRange * lowPercent) + physicalStats.minMax.first;
    const auto max = (physicalValueRange * highPercent) + physicalStats.minMax.first;

    return {min, max};
}

}
