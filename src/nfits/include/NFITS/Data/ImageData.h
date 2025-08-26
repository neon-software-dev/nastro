/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DATA_IMAGEDATA_H
#define NFITS_INCLUDE_NFITS_DATA_IMAGEDATA_H

#include "Data.h"

#include "../SharedLib.h"

#include <vector>
#include <cstdint>
#include <span>
#include <unordered_map>
#include <optional>
#include <utility>

namespace NFITS
{
    class FITSFile;
    class HDU;

    /**
     * Contains axis values which specify a 2D slice of a N-dimension image
     */
    struct ImageSlice
    {
        // Axis values for axis3+. If not set for an axis, slice logic will default the axis value to 0.
        // axisn [3..naxis] -> axis value [0..naxisn)
        std::unordered_map<unsigned int, int64_t> axisValue;
    };

    /**
     * Contains FITS metadata defining the structure of an image
     */
    struct ImageMetadata
    {
        int64_t bitpix;
        std::vector<int64_t> naxisns;
        double bZero = 0.0;
        double bScale = 1.0;
        std::optional<double> dataMin;
        std::optional<double> dataMax;
    };

    struct PhysicalStats
    {
        std::pair<double, double> minMax;

        std::vector<std::size_t> histogram;
        std::vector<std::size_t> histogramCumulative;
    };

    class NFITS_PUBLIC ImageData : public Data
    {
        public:

            ImageData() = default;

            [[nodiscard]] bool LoadFromFileBlocking(const FITSFile* pFile, const HDU* pHDU);

            [[nodiscard]] Type GetType() const override { return Type::Image; }

            [[nodiscard]] const ImageMetadata& GetMetadata() const noexcept { return m_metadata; }

            [[nodiscard]] std::span<const double> GetPhysicalValues() const noexcept { return m_physicalValues; }
            [[nodiscard]] std::optional<std::span<const double>> GetSlicePhysicalValues(const ImageSlice& slice) const;

            [[nodiscard]] std::optional<PhysicalStats> GetSlicePhysicalStats(const ImageSlice& slice) const;
            [[nodiscard]] std::optional<PhysicalStats> GetSliceCubePhysicalStats(const ImageSlice& slice) const;

        private:

            [[nodiscard]] std::optional<uintmax_t> GetSliceIndex(const ImageSlice& slice) const;
            [[nodiscard]] std::optional<uintmax_t> GetSliceCubeIndex(const ImageSlice& slice) const;

        private:

            ImageMetadata m_metadata;
            std::vector<double> m_physicalValues;
            std::vector<PhysicalStats> m_slicePhysicalStats;
            std::vector<PhysicalStats> m_sliceCubePhysicalStats;
    };
}

#endif //NFITS_INCLUDE_NFITS_DATA_IMAGEDATA_H
