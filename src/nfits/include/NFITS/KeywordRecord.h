/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_KEYWORDRECORD_H
#define NFITS_INCLUDE_NFITS_KEYWORDRECORD_H

#include "Def.h"
#include "Error.h"
#include "SharedLib.h"

#include <expected>
#include <array>
#include <cstdint>

namespace NFITS
{
    /**
     * Contains the data specific to one FITS HDU Header Keyword Record
     */
    class NFITS_PUBLIC KeywordRecord
    {
        public:

            [[nodiscard]] static KeywordRecord FromRaw(KeywordRecordCSpan keywordRecordSpan);

        public:

            KeywordRecord() = default;
            explicit KeywordRecord(const KeywordRecordBytes& keywordRecord);

            /**
             * @return The entire keyword record, as a string, including all whitespace, comment, etc
             */
            [[nodiscard]] std::string GetKeywordRecordRaw() const;

            /**
             * @return The keyword name of this keyword record, or std::unexpected if an error occurred
             * when attempting to process they keyword name, or std::nullopt if the keyword record has no
             * keyword name.
             */
            [[nodiscard]] std::expected<std::optional<std::string>, Error> GetKeywordName() const;

            /**
             * @return Whether the keyword record has the '= ' characters indicating the record contains a value
             */
            [[nodiscard]] bool HasValueIndicator() const;

            /**
             * @return The keyword's value as an integer, or std::unexpected if an error occurred (e.g. the keyword
             * doesn't have a value, or the value isn't an integer)
             */
            [[nodiscard]] std::expected<int64_t, Error> GetKeywordValue_AsInteger() const;

            /**
             * @return The keyword's value as a real, or std::unexpected if an error occurred (e.g. the keyword
             * doesn't have a value, or the value isn't a real)
             */
            [[nodiscard]] std::expected<double, Error> GetKeywordValue_AsReal() const;

            /**
             * @return The keyword's value as a logical, or std::unexpected if an error occurred (e.g. the keyword
             * doesn't have a value, or the value isn't a logical)
             */
            [[nodiscard]] std::expected<bool, Error> GetKeywordValue_AsLogical() const;

            /**
             * @return The keyword's value as a string, or std::unexpected if an error occurred (e.g. the keyword
             * doesn't have a value, or the value isn't a string)
             */
            [[nodiscard]] std::expected<std::string, Error> GetKeywordValue_AsString() const;

            /**
             * @return A validation error associated with the keyword record, or std::nullopt if there's no issue
             */
            [[nodiscard]] std::optional<Error> GetValidationError() const;

        private:

            KeywordRecordBytes m_keywordRecord;
    };
}

#endif //NFITS_INCLUDE_NFITS_KEYWORDRECORD_H
