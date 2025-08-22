/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Header.h>

namespace NFITS
{

std::optional<const KeywordRecord*> Header::GetFirstKeywordRecord(const std::string& keywordName) const
{
    for (const auto& headerBlock : headerBlocks)
    {
        for (const auto& keywordRecord : headerBlock.keywordRecords)
        {
            const auto blockKeywordName = keywordRecord.GetKeywordName();
            if (blockKeywordName && (*blockKeywordName == keywordName))
            {
                return &keywordRecord;
            }
        }
    }

    return std::nullopt;
}

std::expected<int64_t, Error> Header::GetFirstKeywordRecord_AsInteger(const std::string& keywordName) const
{
    const auto keywordRecord = GetFirstKeywordRecord(keywordName);
    if (!keywordRecord)
    {
        return std::unexpected(Error::Msg(ErrorType::General, "No such keyword record exists: {}", keywordName));
    }

    return (*keywordRecord)->GetKeywordValue_AsInteger();
}

}
