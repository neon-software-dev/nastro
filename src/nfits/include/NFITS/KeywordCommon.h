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
    static constexpr auto KEYWORD_NAME_PCOUNT = "PCOUNT";
    static constexpr auto KEYWORD_NAME_GCOUNT = "GCOUNT";
    static constexpr auto KEYWORD_NAME_BZERO = "BZERO";
    static constexpr auto KEYWORD_NAME_BSCALE = "BSCALE";
    static constexpr auto KEYWORD_NAME_BLANK = "BLANK";
    static constexpr auto KEYWORD_NAME_BUNIT = "BUNIT";
    static constexpr auto KEYWORD_NAME_DATAMIN = "DATAMIN";
    static constexpr auto KEYWORD_NAME_DATAMAX = "DATAMAX";
    static constexpr auto KEYWORD_NAME_TFORM = "TFORM";
    static constexpr auto KEYWORD_NAME_TTYPE = "TTYPE";
    static constexpr auto KEYWORD_NAME_THEAP = "THEAP";
    static constexpr auto KEYWORD_NAME_TFIELDS = "TFIELDS";
    static constexpr auto KEYWORD_NAME_ZIMAGE = "ZIMAGE";
    static constexpr auto KEYWORD_NAME_ZCMPTYPE = "ZCMPTYPE";
    static constexpr auto KEYWORD_NAME_ZBITPIX = "ZBITPIX";
    static constexpr auto KEYWORD_NAME_ZNAXIS = "ZNAXIS";
    static constexpr auto KEYWORD_NAME_ZTILE = "ZTILE";
    static constexpr auto KEYWORD_NAME_ZSCALE = "ZSCALE";
    static constexpr auto KEYWORD_NAME_ZZERO = "ZZERO";
    static constexpr auto KEYWORD_NAME_ZNAME = "ZNAME";
    static constexpr auto KEYWORD_NAME_ZVAL = "ZVAL";

    // Standard FITS keyword values

    static constexpr auto KEYWORD_VALUE_XTENSION_IMAGE = "IMAGE";
    static constexpr auto KEYWORD_VALUE_XTENSION_TABLE = "TABLE";
    static constexpr auto KEYWORD_VALUE_XTENSION_BINTABLE = "BINTABLE";
}

#endif //NFITS_INCLUDE_NFITS_KEYWORDCOMMON_H
