/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IFITSBYTESOURCE_H
#define NFITS_INCLUDE_NFITS_IFITSBYTESOURCE_H

#include "Bytes.h"
#include "SharedLib.h"
#include "Result.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <expected>

namespace NFITS
{
    static constexpr unsigned int BYTE_SOURCE_TYPE_DISK = 0U;
    static constexpr unsigned int BYTE_SOURCE_TYPE_MEMORY = 1U;

    /**
     * Interface for writing/reading bytes to/from a FITS source
     */
    class NFITS_PUBLIC IFITSByteSource
    {
        public:

            virtual ~IFITSByteSource() = default;

            /**
             * @return An integer representing the specific type of subclass
             */
            [[nodiscard]] virtual unsigned int GetType() const = 0;

            /**
             * @return The bytesize of the FITS source, including all padding and unused space, or
             * std::unexpected upon failure.
             */
            [[nodiscard]] virtual std::expected<ByteSize, Error> GetByteSize() const = 0;

            /**
             * Resize the source to the specified byte size. Any newly added bytes are undefined
             * until written to.
             *
             * @return Whether the source was resized successfully
             */
            virtual Result Resize(const ByteSize& byteSize) = 0;

            /**
             * Read bytes from the FITS source
             *
             * @param dst Destination which receives the read bytes
             * @param byteOffset Byte offset within the source to read from
             * @param byteSize Number of bytes to read from the source and write to dst
             *
             * @return Whether all bytes were read successfully.
             */
            virtual Result ReadBytes(std::span<std::byte> dst, const ByteOffset& byteOffset, const ByteSize& byteSize) = 0;

            /**
             * Write bytes to the FITS source
             *
             * @param src Source for the bytes to be written
             * @param byteOffset Byte offset within the source to write to
             * @param byteSize Number of bytes to read from src and write to the FITS source
             * @param flush Whether the write should be immediately flushed
             *
             * @return Whether all bytes were written successfully.
             */
            virtual Result WriteBytes(std::span<const std::byte> src, const ByteOffset& byteOffset, const ByteSize& byteSize, bool flush) = 0;

            /**
             * Flushes any written bytes which have yet to be flushed.
             *
             * @return Whether the flush was successful
             */
            virtual Result Flush() = 0;
    };
}

#endif //NFITS_INCLUDE_NFITS_IFITSBYTESOURCE_H
