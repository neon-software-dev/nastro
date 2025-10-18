/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Header.h>

namespace NFITS
{

std::optional<KeywordRecord> Header::GetFirstKeywordRecord(const std::string& keywordName) const
{
    for (const auto& headerBlock : headerBlocks)
    {
        for (const auto& keywordRecord : headerBlock.keywordRecords)
        {
            const auto blockKeywordName = keywordRecord.GetKeywordName();
            if (blockKeywordName && (*blockKeywordName == keywordName))
            {
                return keywordRecord;
            }
        }
    }

    return std::nullopt;
}

std::vector<KeywordRecord> Header::GetKeywordsStartingWith(const std::string& keywordNamePrefix) const
{
    std::vector<KeywordRecord> records;

    for (const auto& headerBlock : headerBlocks)
    {
        for (const auto& keywordRecord: headerBlock.keywordRecords)
        {
            const auto blockKeywordName = keywordRecord.GetKeywordName();
            if (blockKeywordName && *blockKeywordName && ((*blockKeywordName)->starts_with(keywordNamePrefix)))
            {
                records.push_back(keywordRecord);
            }
        }
    }

    return records;
}

std::expected<int64_t, Error> Header::GetFirstKeywordRecord_AsInteger(const std::string& keywordName) const
{
    const auto keywordRecord = GetFirstKeywordRecord(keywordName);
    if (!keywordRecord)
    {
        return std::unexpected(Error::Msg("No such keyword record exists: {}", keywordName));
    }

    return keywordRecord->GetKeywordValue_AsInteger();
}

std::expected<double, Error> Header::GetFirstKeywordRecord_AsReal(const std::string& keywordName) const
{
    const auto keywordRecord = GetFirstKeywordRecord(keywordName);
    if (!keywordRecord)
    {
        return std::unexpected(Error::Msg("No such keyword record exists: {}", keywordName));
    }

    return keywordRecord->GetKeywordValue_AsReal();
}

std::expected<bool, Error> Header::GetFirstKeywordRecord_AsLogical(const std::string& keywordName) const
{
    const auto keywordRecord = GetFirstKeywordRecord(keywordName);
    if (!keywordRecord)
    {
        return std::unexpected(Error::Msg("No such keyword record exists: {}", keywordName));
    }

    return keywordRecord->GetKeywordValue_AsLogical();
}

std::expected<std::string, Error> Header::GetFirstKeywordRecord_AsString(const std::string& keywordName) const
{
    const auto keywordRecord = GetFirstKeywordRecord(keywordName);
    if (!keywordRecord)
    {
        return std::unexpected(Error::Msg("No such keyword record exists: {}", keywordName));
    }

    return keywordRecord->GetKeywordValue_AsString();
}

}
