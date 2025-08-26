/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DATA_DATAUTIL_H
#define NFITS_INCLUDE_NFITS_DATA_DATAUTIL_H

#include "Data.h"

#include "../SharedLib.h"

#include <expected>
#include <memory>

namespace NFITS
{
    class FITSFile;
    class HDU;

    [[nodiscard]] NFITS_PUBLIC std::expected<std::unique_ptr<Data>, bool> LoadHDUDataBlocking(const FITSFile* pFile, const HDU* pHDU);
}

#endif //NFITS_INCLUDE_NFITS_DATA_DATAUTIL_H
