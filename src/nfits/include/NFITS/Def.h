/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DEF_H
#define NFITS_INCLUDE_NFITS_DEF_H

#include "Bytes.h"

#include <span>
#include <array>

namespace NFITS
{
    // Common NFITS types definitions

    constexpr auto BLOCK_BYTE_SIZE = ByteSize(2880U);
    constexpr auto KEYWORD_RECORD_BYTE_SIZE = ByteSize(80U);
    constexpr auto KEYWORD_RECORDS_PER_HEADER_BLOCK = 36U;

    using KeywordRecordSpan = std::span<char, KEYWORD_RECORD_BYTE_SIZE.value>;
    using KeywordRecordCSpan = std::span<const char, KEYWORD_RECORD_BYTE_SIZE.value>;
    using KeywordRecordBytes = std::array<char, KEYWORD_RECORD_BYTE_SIZE.value>;

    using KeywordNameSpan = std::span<char, 8>;
    using KeywordNameCSpan = std::span<const char, 8>;

    using KeywordValueIndicatorSpan = std::span<char, 2>;
    using KeywordValueIndicatorCSpan = std::span<const char, 2>;

    using KeywordValueSpan = std::span<char, 70>;
    using KeywordValueCSpan = std::span<const char, 70>;

    using KeywordCommentarySpan = std::span<char, 72>;
    using KeywordCommentaryCSpan = std::span<const char, 72>;
}

#endif //NFITS_INCLUDE_NFITS_DEF_H
