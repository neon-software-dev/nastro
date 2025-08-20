/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_DATA_IMAGEDATA_H
#define SRC_DATA_IMAGEDATA_H

#include "Data.h"

#include <vector>
#include <expected>
#include <memory>
#include <cstddef>
#include <span>
#include <optional>

namespace NFITS
{
    class HDU;
}

namespace Nastro
{
    struct ImageParams
    {
        int64_t bitpix;
        std::vector<int64_t> naxisns;
        std::optional<double> bZero;
        std::optional<double> bScale;
        std::optional<double> dataMin;
        std::optional<double> dataMax;
    };

    class ImageData : public Data
    {
        public:

            [[nodiscard]] static std::expected<std::unique_ptr<ImageData>, bool> FromData(const NFITS::HDU* pHDU, std::vector<std::byte>&& data);

        public:

            ImageData(ImageParams params, std::vector<std::byte> data);

            [[nodiscard]] Type GetType() const override { return Type::Image; }

            [[nodiscard]] const ImageParams& GetParams() const noexcept { return m_params; }
            [[nodiscard]] std::span<const std::byte> GetData() const noexcept { return m_data; }

        private:

            ImageParams m_params;
            std::vector<std::byte> m_data;
    };
}

#endif //SRC_DATA_IMAGEDATA_H
