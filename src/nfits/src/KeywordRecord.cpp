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

    // TODO: Validate value

    return std::nullopt;
}

std::expected<int64_t, Error> KeywordRecord::GetKeywordValue_AsInteger() const
{
    return ParseKeywordValue_AsInteger(m_keywordRecord);
}

std::expected<double, Error> KeywordRecord::GetKeywordValue_AsReal() const
{
    return ParseKeywordValue_AsReal(m_keywordRecord);
}

std::expected<bool, Error> KeywordRecord::GetKeywordValue_AsLogical() const
{
    return ParseKeywordValue_AsLogical(m_keywordRecord);
}

std::expected<std::string, Error> KeywordRecord::GetKeywordValue_AsString() const
{
    return ParseKeywordValue_AsString(m_keywordRecord);
}

}
