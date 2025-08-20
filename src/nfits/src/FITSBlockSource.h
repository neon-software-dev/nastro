/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_FITSBLOCKSOURCE_H
#define NFITS_SRC_FITSBLOCKSOURCE_H

#include "InternalDef.h"

#include <NFITS/Result.h>
#include <NFITS/SharedLib.h>

#include <expected>
#include <memory>

namespace NFITS
{
    class IFITSByteSource;

    /**
     * Wrapper around an IFITSByteSource which allows for reading/writing FITS blocks
     */
    class NFITS_LOCAL FITSBlockSource
    {
        public:

            explicit FITSBlockSource(IFITSByteSource* pByteSource);
            ~FITSBlockSource();

            [[nodiscard]] std::expected<uintmax_t, Error> GetNumBlocks() const;
            Result ResizeBlocks(uintmax_t numBlocks);

            Result ReadBlock(BlockSpan dst, const uintmax_t& blockIndex);
            Result WriteBlock(BlockCSpan src, const uintmax_t& blockIndex, bool flush);

            // Passthrough access to the underlying byte source
            [[nodiscard]] IFITSByteSource* GetByteSource() const noexcept { return m_pByteSource; }

        private:

            IFITSByteSource* m_pByteSource;
    };
}

#endif //NFITS_SRC_FITSBLOCKSOURCE_H
