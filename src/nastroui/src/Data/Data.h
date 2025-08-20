/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef SRC_DATA_DATA_H
#define SRC_DATA_DATA_H

namespace Nastro
{
    class Data
    {
        public:

            enum class Type
            {
                Image
            };

        public:

            virtual ~Data() = default;

            [[nodiscard]] virtual Type GetType() const = 0;
    };
}

#endif //SRC_DATA_DATA_H
