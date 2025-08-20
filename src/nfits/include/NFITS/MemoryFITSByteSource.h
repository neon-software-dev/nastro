/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_MEMORYFITSBYTESOURCE_H
#define NFITS_INCLUDE_NFITS_MEMORYFITSBYTESOURCE_H

#include "SharedLib.h"

#include <NFITS/IFITSByteSource.h>

#include <vector>
#include <cstdint>

namespace NFITS
{
    /**
     * Concrete IFITSByteSource which is backed by CPU memory
     */
    class NFITS_PUBLIC MemoryFITSByteSource : public IFITSByteSource
    {
        public:

            //
            // IFITSByteSource
            //
            [[nodiscard]] unsigned int GetType() const override { return BYTE_SOURCE_TYPE_MEMORY; }
            [[nodiscard]] std::expected<ByteSize, Error> GetByteSize() const override;
            Result Resize(const ByteSize& byteSize) override;

            Result ReadBytes(std::span<std::byte> dst, const ByteOffset& byteOffset, const ByteSize& byteSize) override;
            Result WriteBytes(std::span<const std::byte> src, const ByteOffset& byteOffset, const ByteSize& byteSize, bool flush) override;

            Result Flush() override;

        private:

            std::vector<std::byte> m_data;
    };
}

#endif //NFITS_INCLUDE_NFITS_MEMORYFITSBYTESOURCE_H
