/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_RESULT_H
#define NFITS_INCLUDE_NFITS_RESULT_H

#include "Error.h"

#include <optional>

namespace NFITS
{
    /**
     * NFITS specific Result type, which is in either Success of Fail state, and holds an optional error
     */
    struct Result
    {
        Result() = default;

        explicit Result(std::optional<Error> _error)
            : error(std::move(_error))
        { }

        /**
         * @return A Success result
         */
        [[nodiscard]] static Result Success() { return {}; }

        /**
         * @return A Fail result
         */
        [[nodiscard]] static Result Fail() { return Result(std::nullopt); }

        /**
         * @return A Fail result, with a provided Error
         */
        [[nodiscard]] static Result Fail(Error error) { return Result(error); }

        /**
         * @return A Fail result, with an Error constructed with the provided ErrorType and message
         */
        template<typename... Args>
        [[nodiscard]] static Result Fail(std::string_view rt_fmt_str, Args&&... args)
        {
            return Result(Error::Msg(rt_fmt_str, args...));
        }

        /**
         * @return Whether the result holds a success state
         */
        [[nodiscard]] bool operator()() const { return !error.has_value(); }

        bool operator!() const { return error.has_value(); }

        std::optional<Error> error;
    };
}

#endif //NFITS_INCLUDE_NFITS_RESULT_H
