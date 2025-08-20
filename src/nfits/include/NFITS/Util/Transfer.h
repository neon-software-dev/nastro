/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_UTIL_TRANSFER_H
#define NFITS_INCLUDE_NFITS_UTIL_TRANSFER_H

#include "../SharedLib.h"
#include "../Result.h"

#include <optional>
#include <cstdint>

namespace NFITS
{
    class IFITSByteSource;

    /**
     * Copies the entirety of a source IFITSByteSource to a dest IFITSByteSource, on a per-block basis.
     *
     * Will Resize() the destination before copying, so that it's sized the same as the source.
     *
     * Note that this is a heavy operation.
     *
     * @param pSrc Source IFITSByteSource to read data from
     * @param pDst Destination IFITSByteSource to write data to
     * @param flush Whether to flush after each transferred block
     *
     * @return Whether all blocks were transferred successfully
     */
    NFITS_PUBLIC Result CopyFITSSource(IFITSByteSource* pSrc, IFITSByteSource* pDst, bool flush);
}

#endif //NFITS_INCLUDE_NFITS_UTIL_TRANSFER_H
