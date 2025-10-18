/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_PARSING_H
#define NFITS_SRC_PARSING_H

#include <NFITS/Def.h>
#include <NFITS/Error.h>
#include <NFITS/SharedLib.h>
#include <NFITS/KeywordRecord.h>
#include <NFITS/Data/BinTableData.h>

#include "WCS/WCSInternal.h"

#include <expected>
#include <optional>
#include <cstdint>
#include <string>

namespace NFITS
{
    // Helper funcs for parsing FITS data

    /**
     * Parses a keyword name out of the keyword name portion of a keyword record.
     *
     * @param keywordNameSpan The keyword name span to be parsed
     *
     * @return The keyword name, or std::unexpected upon error when parsing, or std::nullopt if the keyword name
     * doesn't exist (which is valid for some keyword record types)
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<std::optional<std::string>, Error> ParseKeywordName(KeywordNameCSpan keywordNameSpan);

    /**
     * Parses whether a value indicator exists within the value indicator portion of a keyword record.
     *
     * @param valueIndicatorSpan The value indicator span to be parsed
     *
     * @return Whether the value indicator span contains a value indicator ('= ')
     */
    [[nodiscard]] NFITS_PUBLIC bool ParseValueIndicator(KeywordValueIndicatorCSpan valueIndicatorSpan);

    /**
     * Parses a keyword value as an integer value
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<int64_t, Error> ParseKeywordValue_AsInteger(KeywordRecordCSpan keywordRecordSpan);

    /**
     * Parses a keyword value as a real
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<double, Error> ParseKeywordValue_AsReal(KeywordRecordCSpan keywordRecordSpan);

    /**
     * Parses a keyword value as a logical
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<bool, Error> ParseKeywordValue_AsLogical(KeywordRecordCSpan keywordRecordSpan);

    /**
    * Parses a keyword value as a string
    */
    [[nodiscard]] NFITS_PUBLIC std::expected<std::string, Error> ParseKeywordValue_AsString(KeywordRecordCSpan keywordRecordSpan);

    /**
     * Parses a binary table TFORMN keyword value
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<BinFieldForm, Error> ParseBinTable_TFORMN(const std::string& tformn);

    /**
     * Parses a WCS keyword name with form BASENAMEa
     */
    [[nodiscard]] NFITS_PUBLIC std::optional<WCSKeywordName> ParseWCSKeywordName_a(const std::string& keywordName,
                                                                                   const std::string& baseName);

    /**
     * Parses a WCS keyword name with form BASENAMEia
     */
    [[nodiscard]] NFITS_PUBLIC std::optional<WCSKeywordName> ParseWCSKeywordName_ia(const std::string& keywordName,
                                                                                    const std::string& baseName);

    /**
     * Parses a WCS keyword name with form BASENAMEja
     */
    [[nodiscard]] NFITS_PUBLIC std::optional<WCSKeywordName> ParseWCSKeywordName_ja(const std::string& keywordName,
                                                                                    const std::string& baseName);

    /**
     * Parses a WCS keyword name with form BASENAMEi
     */
    [[nodiscard]] NFITS_PUBLIC std::optional<WCSKeywordName> ParseWCSKeywordName_i(const std::string& keywordName,
                                                                                   const std::string& baseName);

    /**
     * Parses a WCS keyword name with form BASENAMEi_ja
     */
    [[nodiscard]] NFITS_PUBLIC std::optional<WCSKeywordName> ParseWCSKeywordName_i_ja(const std::string& keywordName,
                                                                                      const std::string& baseName);

    [[nodiscard]] NFITS_PUBLIC std::expected<WCSCType, Error> ParseWCSCType(const std::string& ctype);
}

#endif //NFITS_SRC_PARSING_H
