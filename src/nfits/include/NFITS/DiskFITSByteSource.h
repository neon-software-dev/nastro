/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DISKFITSBYTESOURCE_H
#define NFITS_INCLUDE_NFITS_DISKFITSBYTESOURCE_H

#include "IFITSByteSource.h"
#include "SharedLib.h"

#include <filesystem>
#include <memory>
#include <expected>
#include <fstream>

namespace NFITS
{
    /**
     * Concrete IFITSByteSource which is backed by a filesystem file
     */
    class NFITS_PUBLIC DiskFITSByteSource : public IFITSByteSource
    {
        public:

            /**
             * Create a DiskFITSByteSource instance by opening a filesystem file
             *
             * @param filePath The file to be opened
             * @param createIfNotExists Whether to create the file if it doesn't already exist
             *
             * @return A DiskFITSByteSource, or an Error on error
             */
            [[nodiscard]] static std::expected<std::unique_ptr<DiskFITSByteSource>, Error> Open(
                const std::filesystem::path& filePath,
                bool createIfNotExists
            );

        private:

            struct Tag{};

        public:

            DiskFITSByteSource(Tag tag, std::filesystem::path filePath);
            ~DiskFITSByteSource() override;

            DiskFITSByteSource(const DiskFITSByteSource&) = delete;
            DiskFITSByteSource& operator=(const DiskFITSByteSource&) = delete;

            [[nodiscard]] std::filesystem::path GetFilesystemPath() const noexcept { return m_filePath; }

            //
            // IFITSByteSource
            //
            [[nodiscard]] unsigned int GetType() const override { return BYTE_SOURCE_TYPE_DISK; }
            [[nodiscard]] std::expected<ByteSize, Error> GetByteSize() const override;
            Result Resize(const ByteSize& byteSize) override;

            Result ReadBytes(std::span<std::byte> dst, const ByteOffset& byteOffset, const ByteSize& byteSize) override;
            Result WriteBytes(std::span<const std::byte> src, const ByteOffset& byteOffset, const ByteSize& byteSize, bool flush) override;

            Result Flush() override;

        private:

            [[nodiscard]] Result OpenStream(bool createIfNotExists);
            void CloseStream();

        private:

            std::filesystem::path m_filePath;

            std::fstream m_stream;
    };
}

#endif //NFITS_INCLUDE_NFITS_DISKFITSBYTESOURCE_H
