/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_WCS_WCSINTERNAL_H
#define NFITS_SRC_WCS_WCSINTERNAL_H

#include <NFITS/Error.h>
#include <NFITS/KeywordRecord.h>

#include <NFITS/WCS/WCSParams.h>

#include <optional>
#include <string>
#include <expected>
#include <variant>

namespace NFITS
{
    struct WCSKeywordName
    {
        std::string name;
        std::string base;
        std::optional<char> a;
        std::optional<int64_t> j;
        std::optional<int64_t> i;
    };

    struct WCSKeywordRecord
    {
        KeywordRecord keywordRecord{};
        WCSKeywordName wcsKeywordName{};
    };

    struct WCSLinearCType
    {
        std::string coordinateType;
    };

    struct WCSNonLinearCType
    {
        std::string coordinateType;
        std::string algorithmCode;
    };

    using WCSCType = std::variant<WCSLinearCType, WCSNonLinearCType>;

    struct HDU;

    /**
     * Parse all image WCS parameters from an HDU's headers
     *
     * @param pHDU The HDU in question
     * @param naxis The NAXIS value for the image stored in the HDU
     *
     * @return The WCS parameters, or std::nullopt if no such keywords exist, or Error upon error parsing keywords
     */
    [[nodiscard]] std::expected<std::optional<WCSParams>, Error> ParseWCSParams(const HDU* pHDU, int64_t naxis);
}

#endif //NFITS_SRC_WCS_WCSINTERNAL_H
