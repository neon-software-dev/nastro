/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_WCS_WCSPARAMS_H
#define NFITS_INCLUDE_NFITS_WCS_WCSPARAMS_H

#include <optional>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>

namespace NFITS
{
    struct WCSDescription
    {
        int64_t wcsAxes{0};
        std::optional<std::string> wcsName;
        int64_t j{0};
        int64_t i{0};

        std::unordered_map<int64_t, std::unordered_map<int64_t, double>> pci_j;
        std::unordered_map<int64_t, double> cdelti;
        std::unordered_map<int64_t, std::unordered_map<int64_t, double>> cdi_j;
        std::unordered_map<int64_t, std::string> ctypei;
        std::unordered_map<int64_t, std::string> cuniti;
        std::unordered_map<int64_t, double> crpixj;
        std::unordered_map<int64_t, double> crvali;
    };

    struct WCSParams
    {
        std::unordered_map<std::optional<char>, WCSDescription> descriptions;
    };
}

#endif //NFITS_INCLUDE_NFITS_WCS_WCSPARAMS_H
