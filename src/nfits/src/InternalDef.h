/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_INTERNALDEF_H
#define NFITS_SRC_INTERNALDEF_H

#include <NFITS/Def.h>

namespace NFITS
{
    //
    // Blocks
    //
    using BlockSpan = std::span<std::byte, BLOCK_BYTE_SIZE.value>;
    using BlockCSpan = std::span<const std::byte, BLOCK_BYTE_SIZE.value>;
    using BlockBytes = std::array<std::byte, BLOCK_BYTE_SIZE.value>;
}

#endif //NFITS_SRC_INTERNALDEF_H
