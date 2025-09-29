/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_ERROR_H
#define NFITS_INCLUDE_NFITS_ERROR_H

#include <string>
#include <format>
#include <optional>

namespace NFITS
{
    /**
     * Custom NFITS error type
     */
    struct Error
    {
        explicit Error(std::optional<std::string> _msg = std::nullopt)
        {
            if (_msg) { msg = *_msg; }
        }

        /**
         * Create an Error from an ErrorType and an associated error message
         */
        template<typename... Args>
        [[nodiscard]] static Error Msg(std::string_view rt_fmt_str, Args&&... args)
        {
            return Error(std::vformat(rt_fmt_str, std::make_format_args(args...)));
        }

        std::string msg;
    };
}

#endif //NFITS_INCLUDE_NFITS_ERROR_H
