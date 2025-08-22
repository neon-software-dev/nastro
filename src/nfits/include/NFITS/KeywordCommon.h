/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_KEYWORDCOMMON_H
#define NFITS_INCLUDE_NFITS_KEYWORDCOMMON_H

namespace NFITS
{
    // Standard FITS keyword names

    static constexpr auto KEYWORD_NAME_SIMPLE = "SIMPLE";
    static constexpr auto KEYWORD_NAME_BITPIX = "BITPIX";
    static constexpr auto KEYWORD_NAME_NAXIS = "NAXIS";
    static constexpr auto KEYWORD_NAME_END = "END";
    static constexpr auto KEYWORD_NAME_XTENSION = "XTENSION";
    static constexpr auto KEYWORD_NAME_BZERO = "BZERO";
    static constexpr auto KEYWORD_NAME_BSCALE = "BSCALE";
    static constexpr auto KEYWORD_NAME_DATAMIN = "DATAMIN";
    static constexpr auto KEYWORD_NAME_DATAMAX = "DATAMAX";

    // Standard FITS keyword values

    static constexpr auto KEYWORD_VALUE_XTENSION_IMAGE = "IMAGE";
    static constexpr auto KEYWORD_VALUE_XTENSION_TABLE = "TABLE";
    static constexpr auto KEYWORD_VALUE_XTENSION_BINTABLE = "BINTABLE";
}

#endif //NFITS_INCLUDE_NFITS_KEYWORDCOMMON_H
