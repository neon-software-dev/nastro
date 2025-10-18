/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "WCSInternal.h"

#include "../Parsing.h"
#include "../Util/Compare.h"

#include <NFITS/HDU.h>

#include <unordered_map>
#include <functional>

namespace NFITS
{

void ParseWCSKeywords(const HDU* pHDU,
                      std::vector<WCSKeywordRecord>& out,
                      const std::string& baseName,
                      const std::function<std::optional<WCSKeywordName>(std::string, std::string)>& decoder)
{
    // Find all keywords that start with baseName
    const auto keywords = pHDU->header.GetKeywordsStartingWith(baseName);

    for (const auto& keyword : keywords)
    {
        const auto keywordName = keyword.GetKeywordName();
        if (keywordName && *keywordName)
        {
            // Pass the keyword name and basename to the decoder to decode the keyword name
            // into a WCSKeywordName
            const auto wcsKeywordName = decoder(**keyword.GetKeywordName(), baseName);
            if (wcsKeywordName)
            {
                out.push_back(WCSKeywordRecord{
                    .keywordRecord = keyword,
                    .wcsKeywordName = *wcsKeywordName
                });
            }
        }
    }
}

static constexpr auto ParseWCSFunc_a = [](const auto& keywordName, const auto& baseName){
    return ParseWCSKeywordName_a(keywordName, baseName);
};

static constexpr auto ParseWCSFunc_i_ja = [](const auto& keywordName, const auto& baseName){
    return ParseWCSKeywordName_i_ja(keywordName, baseName);
};

static constexpr auto ParseWCSFunc_ia = [](const auto& keywordName, const auto& baseName){
    return ParseWCSKeywordName_ia(keywordName, baseName);
};

static constexpr auto ParseWCSFunc_ja = [](const auto& keywordName, const auto& baseName){
    return ParseWCSKeywordName_ja(keywordName, baseName);
};

struct WCSKeywordRecords
{
    // Base keyword name -> Records matching that base
    std::unordered_map<std::string, std::vector<WCSKeywordRecord>> byBase;

    // Full keyword name -> Record for that name
    std::unordered_map<std::string, WCSKeywordRecord> byName;
};

template<typename T>
std::expected<T , Error> GetWCSKeywordValue(const std::optional<char>& a,
                                            const WCSKeywordRecords& wcsKeywordRecords,
                                            std::string keywordName,
                                            const T& defaultValue)
{
    if (a) { keywordName += *a; }

    const auto it = wcsKeywordRecords.byName.find(keywordName);
    if (it != wcsKeywordRecords.byName.cend())
    {
        return it->second.keywordRecord.GetKeywordValue<T>();
    }
    else
    {
        return defaultValue;
    }
}

std::expected<WCSDescription, Error> ToWCSDescription(const int64_t& naxis,
                                                      const std::optional<char>& a,
                                                      const WCSKeywordRecords& wcsKeywordRecords)
{
    WCSDescription description{};

    //
    // Loop over all wcs keywords and determine max 'j' and 'i' values that are in use
    //
    int64_t maxj = 0;
    int64_t maxi = 0;

    for (const auto& it : wcsKeywordRecords.byName)
    {
        if (it.second.wcsKeywordName.j) { maxj = std::max(maxj, *it.second.wcsKeywordName.j); }
        if (it.second.wcsKeywordName.i) { maxi = std::max(maxi, *it.second.wcsKeywordName.i); }
    }

    description.j = maxj;
    description.i = maxi;

    //
    // Copy WCS keyword values into the WCSDescription fields
    //

    // WCSAXESa
    // "integer; default: NAXIS, or larger of WCS indices i or j"
    {
        int64_t defaultWCSAXES = naxis;
        defaultWCSAXES = std::max(defaultWCSAXES, maxi);
        defaultWCSAXES = std::max(defaultWCSAXES, maxj);

        const auto val = GetWCSKeywordValue<int64_t>(a, wcsKeywordRecords, std::format("WCSAXES"), defaultWCSAXES);
        if (!val) { return std::unexpected(val.error()); }
        description.wcsAxes = *val;
    }

    // WCSNAMEa
    {
        // "string; default for a: ' ' (i.e., blank, for the primary WCS, else a character A through Z that
        // specifies the coordinate version"
        const auto defaultWCSNAME = a ? std::string{*a} : " ";

        const auto val = GetWCSKeywordValue<std::string>(a, wcsKeywordRecords, std::format("WCSNAME"), defaultWCSNAME);
        if (!val) { return std::unexpected(val.error()); }
        description.wcsName = *val;
    }

    //
    // Process keyword records containing 'i' component in their name
    //
    for (int64_t i = 1; i <= maxi; ++i)
    {
        // CDELTia
        if (wcsKeywordRecords.byBase.contains("CDELT"))
        {
            // "floating point; indexed; default: 1.0"
            const auto val = GetWCSKeywordValue<double>(a, wcsKeywordRecords, std::format("CDELT{}", i), 1.0);
            if (!val) { return std::unexpected(val.error()); }
            description.cdelti[i] = *val;

            // "The value must not be zero"
            if (AreEqual(description.cdelti[i], 0.0))
            {
                return std::unexpected(Error::Msg("CDELTi keyword has invalid value of 0.0"));
            }
        }

        // CTYPEia
        if (wcsKeywordRecords.byBase.contains("CTYPE"))
        {
            // "string; indexed; default: ' ' (i.e. a linear, undefined axis)"
            const auto val = GetWCSKeywordValue<std::string>(a, wcsKeywordRecords, std::format("CTYPE{}", i), " ");
            if (!val) { return std::unexpected(val.error()); }
            description.ctypei[i] = *val;
        }

        // CUNITia
        if (wcsKeywordRecords.byBase.contains("CUNIT"))
        {
            // "string; indexed; default: ' ' (i.e., undefined)"
            const auto val = GetWCSKeywordValue<std::string>(a, wcsKeywordRecords, std::format("CUNIT{}", i), " ");
            if (!val) { return std::unexpected(val.error()); }
            description.cuniti[i] = *val;
        }

        // CRVALia
        if (wcsKeywordRecords.byBase.contains("CRVAL"))
        {
            // "floating point; indexed; default: 0.0"
            const auto val = GetWCSKeywordValue<double>(a, wcsKeywordRecords, std::format("CRVAL{}", i), 0.0);
            if (!val) { return std::unexpected(val.error()); }
            description.crvali[i] = *val;
        }
    }

    //
    // Process keyword records containing 'j' component in their name
    //
    for (int64_t j = 1; j <= maxj; ++j)
    {
        // CRPIXja
        if (wcsKeywordRecords.byBase.contains("CRPIX"))
        {
            // "floating point; indexed; default: 0.0"
            const auto val = GetWCSKeywordValue<double>(a, wcsKeywordRecords, std::format("CRPIX{}", j), 0.0);
            if (!val) { return std::unexpected(val.error()); }
            description.crpixj[j] = *val;
        }
    }

    //
    // Process keyword records containing both 'i' and 'j' component in their name
    //
    for (int64_t i = 1; i <= maxi; ++i)
    {
        for (int64_t j = 1; j <= maxj; ++j)
        {
            // PCi_ja
            if (wcsKeywordRecords.byBase.contains("PC"))
            {
                // "floating point; defaults: 1.0 when i = j, 0.0 otherwise"
                const auto val = GetWCSKeywordValue<double>(a, wcsKeywordRecords, std::format("PC{}_{}", i, j), (i == j) ? 1.0 : 0.0);
                if (!val) { return std::unexpected(val.error()); }
                description.pci_j[i][j] = *val;
            }

            // CDi_ja
            if (wcsKeywordRecords.byBase.contains("CD"))
            {
                // "floating point; defaults: 0.0"
                const auto val = GetWCSKeywordValue<double>(a, wcsKeywordRecords, std::format("CD{}_{}", i, j), 0.0);
                if (!val) { return std::unexpected(val.error()); }
                description.cdi_j[i][j] = *val;
            }
        }
    }

    return description;
}

std::expected<std::optional<WCSParams>, Error> ParseWCSParams(const HDU* pHDU, int64_t naxis)
{
    //
    // Parse all WCS keyword records in the HDU into WCSKeywordRecord objects
    //
    using ParseWCSFunc = std::function<std::optional<WCSKeywordName>(std::string, std::string)>;

    const auto wcsBaseNames = std::unordered_map<std::string, ParseWCSFunc> {
        {"WCSAXES", ParseWCSFunc_a},
        {"WCSNAME", ParseWCSFunc_a},
        {"PC", ParseWCSFunc_i_ja},
        {"CD", ParseWCSFunc_i_ja},
        {"CDELT", ParseWCSFunc_ia},
        {"CTYPE", ParseWCSFunc_ia},
        {"CUNIT", ParseWCSFunc_ia},
        {"CRVAL", ParseWCSFunc_ia},
        {"CRPIX", ParseWCSFunc_ja},
    };

    std::vector<WCSKeywordRecord> wcsKeywordRecords;

    for (const auto& it : wcsBaseNames)
    {
        ParseWCSKeywords(pHDU, wcsKeywordRecords, it.first, it.second);
    }

    //
    // Group the WCS keyword records by alternative code 'a' (note that it's valid for 'a' to be std::nullopt
    // in the scenario where there's no alternative codes)
    //
    std::unordered_map<std::optional<char>, WCSKeywordRecords> byA;

    for (const auto& wcsKeywordRecord : wcsKeywordRecords)
    {
        // Ignoring the keyword record if we've already seen a record with the same exact name. This is to somewhat
        // gracefully handle invalid FITS files with the same keyword defined multiple times; only use the first
        // definition if so.
        if (!byA[wcsKeywordRecord.wcsKeywordName.a].byName.contains(wcsKeywordRecord.wcsKeywordName.name))
        {
            byA[wcsKeywordRecord.wcsKeywordName.a].byBase[wcsKeywordRecord.wcsKeywordName.base].push_back(wcsKeywordRecord);
            byA[wcsKeywordRecord.wcsKeywordName.a].byName[wcsKeywordRecord.wcsKeywordName.name] = wcsKeywordRecord;
        }
    }

    //
    // Transform WCS keyword records for each 'a' into a WCSDescription object
    //
    WCSParams params{};

    for (const auto& it : byA)
    {
        const auto description = ToWCSDescription(naxis, it.first, it.second);
        if (!description)
        {
            return std::unexpected(description.error());
        }

        params.descriptions.insert({it.first, *description});
    }

    return params;
}

}
