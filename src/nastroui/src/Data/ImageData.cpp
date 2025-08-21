/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "ImageData.h"

#include <NFITS/HDU.h>
#include <NFITS/KeywordName.h>

namespace Nastro
{

std::optional<double> GetOptionalDoubleKeywordValue(const NFITS::HDU* pHDU, const std::string& keywordName)
{
    const auto keyword = pHDU->header.GetFirstKeywordRecord(keywordName);
    if (keyword)
    {
        const auto val = (*keyword)->GetKeywordValueAsReal();
        if (val)
        {
            return *val;
        }
    }

    return std::nullopt;
}

std::expected<std::unique_ptr<ImageData>, bool> ImageData::FromData(const NFITS::HDU *pHDU, std::vector<std::byte> &&data)
{
    const auto bitpix = pHDU->header.GetFirstKeywordRecordAsInteger(NFITS::KEYWORD_NAME_BITPIX);
    if (!bitpix) { return std::unexpected(false); }

    const auto naxis = pHDU->header.GetFirstKeywordRecordAsInteger(NFITS::KEYWORD_NAME_NAXIS);
    if (!naxis) { return std::unexpected(false); }

    std::vector<int64_t> naxisns;
    for (int64_t n = 1; n <= *naxis; ++n)
    {
        const auto naxisn = pHDU->header.GetFirstKeywordRecordAsInteger(std::format("NAXIS{}", n));
        if (!naxisn) { return std::unexpected(false); }

        naxisns.push_back(*naxisn);
    }

    const auto bZero = GetOptionalDoubleKeywordValue(pHDU, NFITS::KEYWORD_NAME_BZERO);
    const auto bScale = GetOptionalDoubleKeywordValue(pHDU, NFITS::KEYWORD_NAME_BSCALE);

    ImageParams params{};
    params.bitpix = *bitpix;
    params.naxisns = naxisns;
    if (bZero) { params.bZero = *bZero; }
    if (bScale) { params.bScale = *bScale; }
    params.dataMin = GetOptionalDoubleKeywordValue(pHDU, NFITS::KEYWORD_NAME_DATAMIN);
    params.dataMax = GetOptionalDoubleKeywordValue(pHDU, NFITS::KEYWORD_NAME_DATAMAX);

    return std::make_unique<ImageData>(params, std::move(data));
}

ImageData::ImageData(ImageParams params,
                     std::vector<std::byte> data)
    : m_params(std::move(params))
    , m_data(std::move(data))
{

}

}
