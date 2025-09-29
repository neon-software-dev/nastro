/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DATA_DATA_H
#define NFITS_INCLUDE_NFITS_DATA_DATA_H

#include "../SharedLib.h"

namespace NFITS
{
    class NFITS_PUBLIC Data
    {
        public:

            enum class Type
            {
                Image,
                BinTable
            };

        public:

            virtual ~Data() = default;

            [[nodiscard]] virtual Type GetType() const = 0;
    };
}

#endif //NFITS_INCLUDE_NFITS_DATA_DATA_H
