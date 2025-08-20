/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_DATA_DATAUTIL_H
#define SRC_DATA_DATAUTIL_H

#include "Data.h"

#include <NFITS/FITSFile.h>

#include <expected>
#include <memory>

namespace Nastro
{
    [[nodiscard]] std::expected<std::unique_ptr<Data>, bool> LoadHDUDataBlocking(const NFITS::FITSFile* pFile, const NFITS::HDU* pHDU);
}

#endif //SRC_DATA_DATAUTIL_H
