/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_COMMON_H
#define SRC_UTIL_COMMON_H

#include <NFITS/WCS/WCS.h>

#include <unordered_set>
#include <string>
#include <filesystem>
#include <utility>
#include <vector>
#include <optional>

namespace Nastro
{
    static const std::unordered_set<std::string> VALID_FITS_EXTENSIONS = { ".fts", ".fits", ".fit" };

    struct FileHDU
    {
        std::filesystem::path filePath;
        uintmax_t hduIndex;

        auto operator<=>(const FileHDU&) const = default;
    };

    struct PixelDetails
    {
        // FITS-standard space coordinate of the pixel (1,1 is center of first pixel)
        std::vector<double> pixelCoordinate;

        // Physical value/unit associated with the pixel's underlying data
        double physicalValue{0.0};
        std::optional<std::string> physicalUnit;

        // WCS coordinates derived from the pixel's coordinate
        std::vector<NFITS::WCSWorldCoord> wcsCoords;
    };
}

#endif //SRC_UTIL_COMMON_H
