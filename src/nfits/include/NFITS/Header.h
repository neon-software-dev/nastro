/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_HEADER_H
#define NFITS_INCLUDE_NFITS_HEADER_H

#include "HeaderBlock.h"
#include "SharedLib.h"

#include <vector>
#include <optional>

namespace NFITS
{
    /**
     * Contains the data for all header blocks related to a particular HDU
     */
    struct NFITS_PUBLIC Header
    {
        /**
         * Helper method which looks through all header blocks and returns the first KeywordRecord it finds matching
         * the provided keyword name.
         *
         * @param keywordName The keyword name to search for
         *
         * @return The first KeywordRecord found, or std::nullopt if no such keyword record exists
         */
        [[nodiscard]] std::optional<const KeywordRecord*> GetFirstKeywordRecord(const std::string& keywordName) const;

        /**
         * Helper method which looks through all header blocks and returns the first KeywordRecord it finds matching
         * the provided keyword name, interpreted as an integer.
         *
         * @param keywordName The keyword name to search for
         *
         * @return The integer value of the first matching keyword record, or std::nullopt if no such keyword record exists
         */
        [[nodiscard]] std::expected<int64_t, Error> GetFirstKeywordRecordAsInteger(const std::string& keywordName) const;

        /**
         * The header blocks that define this Header
         */
        std::vector<HeaderBlock> headerBlocks;
    };
}

#endif //NFITS_INCLUDE_NFITS_HEADER_H
