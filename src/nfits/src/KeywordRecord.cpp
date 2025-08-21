/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/KeywordRecord.h>

#include "Parsing.h"

#include <algorithm>
#include <unordered_set>

namespace NFITS
{

KeywordRecord KeywordRecord::FromRaw(KeywordRecordCSpan keywordRecordSpan)
{
    KeywordRecordBytes keywordRecord{0};
    std::copy_n(keywordRecordSpan.begin(), KEYWORD_RECORD_BYTE_SIZE.value, keywordRecord.begin());

    return KeywordRecord(keywordRecord);
}

KeywordRecord::KeywordRecord(const KeywordRecordBytes& keywordRecord)
    : m_keywordRecord(keywordRecord)
{

}

std::string KeywordRecord::GetKeywordRecordRaw() const
{
    const auto charSpan = std::span<const char>(m_keywordRecord);
    return {charSpan.begin(), charSpan.end()};
}

std::expected<std::optional<std::string>, Error> KeywordRecord::GetKeywordName() const
{
    return ParseKeywordName(std::span<const char>(m_keywordRecord).subspan<0, 8>());
}

bool KeywordRecord::IsMandatoryKeywordName() const
{
    const auto keywordNameExpect = GetKeywordName();

    // If failed to parse keyword name, not a mandatory keyword
    if (!keywordNameExpect) { return false; }

    // If there is no keyword name, not a mandatory keyword
    if (!*keywordNameExpect) { return false; }

    const auto keywordName = **keywordNameExpect;

    // TODO! Others
    static std::unordered_set<std::string> MANDATORY_KEYWORD_NAMES = {
        "SIMPLE",
        "BITPIX",
        "NAXIS",
        "END"
    };

    if (MANDATORY_KEYWORD_NAMES.contains(keywordName))
    {
        return true;
    }

    // Special case handling for NAXISn dynamic keyword name
    if (keywordName.starts_with("NAXIS") && keywordName.length() > 5)
    {
        /**
         * [4.4.1.1.]
         * NAXIS keyword. The value field shall contain a
         * non-negative integer no greater than 999
         * [..]
         * The NAXISn keywords must be present for
         * all values n = 1, . . . , NAXIS
         */

        // n-part of the keyword
        const auto nStr = keywordName.substr(5);

        static auto IS_DIGIT_CHAR = [](char c){ return '0' <= c && c <= '9'; };

        // Extra chars after NAXIS must be digits
        const bool allDigitChars = std::ranges::all_of(nStr, [](char c){
            return IS_DIGIT_CHAR(c);
        });

        // If all digits, and digits between 0..999, its an NAXISn keyword
        if (allDigitChars && (nStr.length() <= 3))
        {
            return true;
        }
    }

    return false;
}

bool KeywordRecord::HasValueIndicator() const
{
    return ParseValueIndicator(std::span<const char>(m_keywordRecord).subspan<8, 2>());
}

std::optional<Error> KeywordRecord::GetValidationError() const
{
    const auto keywordName = GetKeywordName();
    if (!keywordName)
    {
        return keywordName.error();
    }

    // TODO! Validate value

    return std::nullopt;
}

std::expected<int64_t, Error> KeywordRecord::GetKeywordValueAsInteger() const
{
    return ParseKeywordValue_AsInteger(
        std::span<const char>(m_keywordRecord).subspan<10, 70>(),
        IsMandatoryKeywordName()
    );
}

std::expected<double, Error> KeywordRecord::GetKeywordValueAsReal() const
{
    return ParseKeywordValue_AsReal(
        std::span<const char>(m_keywordRecord).subspan<10, 70>(),
        IsMandatoryKeywordName()
    );
}

}
