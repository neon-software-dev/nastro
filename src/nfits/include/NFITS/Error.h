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
    enum class ErrorType
    {
        General,
        Parse,
        Validation
    };

    /**
     * Custom NFITS error type
     */
    struct Error
    {
        Error() = default;

        explicit Error(ErrorType _type, std::optional<std::string> _msg = std::nullopt)
            : isSet(true)
            , type(_type)
        {
            if (_msg) { msg = *_msg; }
        }

        /**
         * @return Whether this object holds an error
         */
        [[nodiscard]] bool operator()() const { return isSet; }

        bool operator!() const { return !isSet; }

        /**
         * Create an Error from an ErrorType, with no message attached
         */
        [[nodiscard]] static Error Type(ErrorType type) { return Error{type, std::nullopt}; }

        /**
         * Create an Error from an ErrorType and an associated error message
         */
        template<typename... Args>
        [[nodiscard]] static Error Msg(ErrorType type, std::string_view rt_fmt_str, Args&&... args)
        {
            return Error(type, std::vformat(rt_fmt_str, std::make_format_args(args...)));
        }

        bool isSet{false};
        ErrorType type{ErrorType::General};
        std::string msg;
    };
}

#endif //NFITS_INCLUDE_NFITS_ERROR_H
