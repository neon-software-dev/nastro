/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Util/Transfer.h>
#include <NFITS/IFITSByteSource.h>
#include <NFITS/Def.h>

#include "../FITSBlockSource.h"

#include <array>

namespace NFITS
{

Result CopyFITSSource(IFITSByteSource* pSrc, IFITSByteSource* pDst, bool flush)
{
    // Wrap the byte sources with block sources to work with them on a per-block basis
    FITSBlockSource blockSrc(pSrc);
    FITSBlockSource blockDst(pDst);

    const auto sourceBlocks = blockSrc.GetNumBlocks();
    if (!sourceBlocks)
    {
        return Result::Fail(sourceBlocks.error());
    }

    // Resize the destination to match the source's block count
    auto result = blockDst.ResizeBlocks(*sourceBlocks);
    if (!result)
    {
        return result;
    }

    BlockBytes blockBytes{};

    for (uintmax_t blockIndex = 0; blockIndex < *sourceBlocks; ++blockIndex)
    {
        result = blockSrc.ReadBlock(blockBytes, blockIndex);
        if (!result) { return result; }

        result = blockDst.WriteBlock(blockBytes, blockIndex, flush);
        if (!result) { return result; }
    }

    return Result::Success();
}

}