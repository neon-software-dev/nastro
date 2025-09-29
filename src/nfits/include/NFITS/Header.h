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

        //
        // Helper methods which look through all header blocks and returns the first KeywordRecord it finds matching
        // the provided keyword name, interpreted as the specified type.
        [[nodiscard]] std::expected<int64_t, Error> GetFirstKeywordRecord_AsInteger(const std::string& keywordName) const;
        [[nodiscard]] std::expected<double, Error> GetFirstKeywordRecord_AsReal(const std::string& keywordName) const;
        [[nodiscard]] std::expected<bool, Error> GetFirstKeywordRecord_AsLogical(const std::string& keywordName) const;
        [[nodiscard]] std::expected<std::string, Error> GetFirstKeywordRecord_AsString(const std::string& keywordName) const;

        /**
         * The header blocks that define this Header
         */
        std::vector<HeaderBlock> headerBlocks;
    };
}

#endif //NFITS_INCLUDE_NFITS_HEADER_H
