/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DATA_IMAGEDATA_H
#define NFITS_INCLUDE_NFITS_DATA_IMAGEDATA_H

#include "Data.h"

#include "../SharedLib.h"
#include "../Error.h"

#include "../Image/ImageSliceSource.h"

#include <unordered_map>
#include <cstdint>
#include <optional>
#include <vector>
#include <span>
#include <memory>
#include <expected>

namespace NFITS
{
    class FITSFile;
    struct HDU;

    class NFITS_PUBLIC ImageData : public Data, public ImageSliceSource
    {
        public:

            ImageData() = default;
            ~ImageData() override = default;

            [[nodiscard]] static std::expected<std::unique_ptr<ImageData>, Error> LoadFromFileBlocking(const FITSFile* pFile, const HDU* pHDU);

            //
            // Data
            //
            [[nodiscard]] Type GetType() const override { return Type::Image; }

            //
            // ImageSliceSource
            //
            [[nodiscard]] ImageSliceSpan GetImageSliceSpan() const override { return m_sliceSpan; }
            [[nodiscard]] std::optional<ImageSlice> GetImageSlice(const ImageSliceKey& sliceKey) const override;

        private:

            [[nodiscard]] std::optional<uintmax_t> GetSliceIndex(const ImageSliceKey& sliceKey) const;
            [[nodiscard]] std::optional<uintmax_t> GetSliceCubeIndex(const ImageSliceKey& sliceKey) const;

        private:

            ImageSliceSpan m_sliceSpan;
            std::vector<double> m_physicalValues;
            std::vector<PhysicalStats> m_slicePhysicalStats;
            std::vector<PhysicalStats> m_sliceCubePhysicalStats;
    };
}

#endif //NFITS_INCLUDE_NFITS_DATA_IMAGEDATA_H
