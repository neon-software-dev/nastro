/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_VALIDATION_H
#define NFITS_SRC_VALIDATION_H

#include <NFITS/Header.h>
#include <NFITS/Result.h>
#include <NFITS/SharedLib.h>

namespace NFITS
{
    // Helper functions for validating the contents of FITS files

    [[nodiscard]] NFITS_PUBLIC Result ValidatePrimaryHeader(const Header& header);
}

#endif //NFITS_SRC_VALIDATION_H
