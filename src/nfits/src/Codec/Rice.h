/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_CODEC_RICE_H
#define NFITS_SRC_CODEC_RICE_H

#include <NFITS/Error.h>

#include <span>
#include <expected>
#include <vector>
#include <cstdint>

namespace NFITS
{
    /**
     * Provides RICE_1 image codec
     */
    class RiceCodec
    {
        public:

            explicit RiceCodec(unsigned int blockSize);

            /**
             * Decompress RICE_1 data, transformed into a double.
             *
             * @param bytepix Bytesize of the underlying data type
             * @param compressed Span providing the compressed data
             * @param outputSize Number of elements in the uncompressed output
             *
             * @return The uncompressed data, or Error upon error
             */
            [[nodiscard]] std::expected<std::vector<double>, Error> Decompress(
                int64_t bytepix,
                std::span<const std::byte> compressed,
                std::size_t outputSize) const;

            [[nodiscard]] std::expected<std::vector<uint32_t>, Error> Decompress_32(
                std::span<const std::byte> compressed,
                std::size_t outputSize) const;

        private:

            unsigned int m_blockSize;
    };
}

#endif //NFITS_SRC_CODEC_RICE_H
