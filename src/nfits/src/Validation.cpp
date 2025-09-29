/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "Validation.h"

#include <NFITS/KeywordCommon.h>

#include <algorithm>

namespace NFITS
{

bool HeaderBlockContainsKeywordName(const HeaderBlock& headerBlock, const std::string& keywordName)
{
    return std::ranges::any_of(headerBlock.keywordRecords, [&](const auto& keywordRecord){
        return !keywordRecord.GetValidationError() && (*keywordRecord.GetKeywordName() == keywordName);
    });
}

bool HeaderBlockContainsKeywordName(const HeaderBlock& headerBlock, const std::string& keywordName, const unsigned int keywordPosition)
{
    if (keywordPosition >= headerBlock.keywordRecords.size())
    {
        return false;
    }

    const auto& keywordRecord = headerBlock.keywordRecords.at(keywordPosition);

    return !keywordRecord.GetValidationError() && (*keywordRecord.GetKeywordName() == keywordName);
}

bool HeaderContainsKeywordName(const Header& header, const std::string& keywordName)
{
    return std::ranges::any_of(header.headerBlocks, [&](const auto& headerBlock){
        return HeaderBlockContainsKeywordName(headerBlock, keywordName);
    });
}

Result ValidatePrimaryHeader(const Header& header)
{
    // [3.2]
    // "The primary HDU and every extension HDU shall consist of
    // one or more 2880-byte header blocks[..]"
    if (header.headerBlocks.empty())
    {
        return Result::Fail("ValidatePrimaryHeader: Header must contain one or more header blocks");
    }

    // [4.4.1.1.]
    // "The SIMPLE keyword is required to be the first keyword
    // in the primary header of all FITS files"
    //
    // [Table 7]
    // "Mandatory keywords for primary header"
    // "SIMPLE, BITPIX, NAXIS, NAXISn, n = 1, . . . , NAXIS, END
    const auto& firstHeaderBlock = header.headerBlocks.at(0);

    if (!HeaderBlockContainsKeywordName(firstHeaderBlock, KEYWORD_NAME_SIMPLE, 0U))
    {
        return Result::Fail("ValidatePrimaryHeader: First keyword must be the SIMPLE keyword");
    }

    if (!HeaderBlockContainsKeywordName(firstHeaderBlock, KEYWORD_NAME_BITPIX, 1U))
    {
        return Result::Fail("ValidatePrimaryHeader: Second keyword must be the BITPIX keyword");
    }

    if (!HeaderBlockContainsKeywordName(firstHeaderBlock, KEYWORD_NAME_NAXIS, 2U))
    {
        return Result::Fail("ValidatePrimaryHeader: Third keyword must be the NAXIS keyword");
    }

    const auto naxisValue = firstHeaderBlock.keywordRecords.at(2).GetKeywordValue_AsInteger();
    if (!naxisValue)
    {
        return Result::Fail("ValidatePrimaryHeader: NAXIS keyword failed parsing");
    }

    // [4.4.1.1.]
    // "The value field shall contain a non-negative
    // integer no greater than 999"
    if (!(*naxisValue > 0 && *naxisValue <= 999))
    {
        return Result::Fail("ValidatePrimaryHeader: NAXIS value out of range: {}", *naxisValue);
    }

    for (unsigned int naxis = 0; naxis < *naxisValue; ++naxis)
    {
        // 'NAXISn, n = 1, . . . , NAXIS"
        const auto naxisnKeywordName = std::format("{}{}", KEYWORD_NAME_NAXIS, naxis + 1);

        // 3U because the position of the root NAXIS keyword is 2U, and NAXISn keywords must directly follow it, in order
        const auto naxisnKeywordIndex = 3U + naxis;

        if (!HeaderBlockContainsKeywordName(firstHeaderBlock, naxisnKeywordName, naxisnKeywordIndex))
        {
            return Result::Fail("ValidatePrimaryHeader: Failed to find properly positioned {} keyword", naxisnKeywordName);
        }

        const auto naxisnValue = firstHeaderBlock.keywordRecords.at(naxisnKeywordIndex).GetKeywordValue_AsInteger();
        if (!naxisnValue)
        {
            return Result::Fail("ValidatePrimaryHeader: {} keyword failed parsing", naxisnKeywordName);
        }
    }

    return Result::Success();
}

}
