/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_FITSFILE_H
#define NFITS_INCLUDE_NFITS_FITSFILE_H

#include "Result.h"
#include "SharedLib.h"

#include <NFITS/HDU.h>

#include <memory>
#include <expected>
#include <vector>
#include <optional>

namespace NFITS
{
    class IFITSByteSource;

    /**
     * Provides access to a FITS file via a provided IFITSByteSource
     */
    class NFITS_PUBLIC FITSFile
    {
        public:

            /**
             * Opens a FITS file contained within the provided IFITSByteSource. Reads HDU blocks to retrieve overall
             * FITS file metadata, but does not read any data blocks.
             *
             * Note that this function only does the bare minimum of validation needed to understand the overall file
             * structure; it does not perform any extra validation for whether the file's contents are valid.
             *
             * @return A valid FITSFile, or an Error on failure to open.
             */
            [[nodiscard]] static std::expected<std::unique_ptr<FITSFile>, Error> OpenBlocking(std::unique_ptr<IFITSByteSource> pSource);

        public:

            struct Tag{};

        public:

            FITSFile(Tag, std::unique_ptr<IFITSByteSource> pSource, std::vector<HDU> hdus);
            ~FITSFile();

            [[nodiscard]] IFITSByteSource* GetByteSource() const noexcept { return m_pSource.get(); }

            [[nodiscard]] std::size_t GetNumHDUs() const noexcept { return m_hdus.size(); }
            [[nodiscard]] std::optional<const HDU*> GetHDU(uintmax_t index) const;

        private:

            std::unique_ptr<IFITSByteSource> m_pSource;
            std::vector<HDU> m_hdus;
    };
}

#endif //NFITS_INCLUDE_NFITS_FITSFILE_H
