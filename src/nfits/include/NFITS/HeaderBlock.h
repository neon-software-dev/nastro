/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_HEADERBLOCK_H
#define NFITS_INCLUDE_NFITS_HEADERBLOCK_H

#include "Def.h"

#include "KeywordRecord.h"

#include <array>

namespace NFITS
{
    /**
     * Contains data for one specific HDU header block
     */
    struct HeaderBlock
    {
        std::array<KeywordRecord, KEYWORD_RECORDS_PER_HEADER_BLOCK> keywordRecords{};
    };
}

#endif //NFITS_INCLUDE_NFITS_HEADERBLOCK_H
