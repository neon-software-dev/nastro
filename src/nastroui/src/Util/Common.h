/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_UTIL_COMMON_H
#define SRC_UTIL_COMMON_H

#include <unordered_set>
#include <string>
#include <filesystem>

namespace Nastro
{
    static const std::unordered_set<std::string> VALID_FITS_EXTENSIONS = { ".fts", ".fits", ".fit" };

    struct FileHDU
    {
        std::filesystem::path filePath;
        uintmax_t hduIndex;

        auto operator<=>(const FileHDU&) const = default;
    };
}

#endif //SRC_UTIL_COMMON_H
