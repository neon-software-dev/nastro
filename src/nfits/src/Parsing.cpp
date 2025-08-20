/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "Parsing.h"

#include <algorithm>

namespace NFITS
{

static constexpr auto IsUpperCaseAlphabetic = [](const char& c){ return 'A' <= c && c <= 'Z'; };
static constexpr auto IsDigit = [](const char& c){ return '0' <= c && c <= '9'; };
static constexpr auto IsASCIITextChar = [](const char& c){ return 32 <= c && c <= 126; };
static constexpr auto IsSpaceChar = [](const char& c){ return c == ' '; };
static constexpr auto IsSignChar = [](const char& c){ return (c == '+') || (c == '-'); };
static constexpr auto IsDecimalPointChar = [](const char& c){ return c == '.'; };
static constexpr auto IsExponentChar = [](const char& c){ return (c == 'E') || (c == 'D'); };

/**
 * Parses a keyword name span and returns the parsed keyword name.
 *
 * Returns std::nullopt for blank (all spaces) keyword names. Returns std::unexpected on any parse error.
 */
std::expected<std::optional<std::string>, Error> ParseKeywordName(KeywordNameCSpan keywordNameSpan)
{
    /**
     * [4.1.1.]
     * Each 80-character header keyword record shall consist of a key-word name
     *
     * [4.1.2.1.]
     * "The keyword name shall be a left justified, eight-character,
     * space-filled, ASCII string with no embedded spaces. All digits
     * 0 through 9 (decimal ASCII codes 48 to 57, or hexadecimal 30
     * to 39) and upper case Latin alphabetic characters ’A’ through
     * ’Z’ (decimal 65 to 90 or hexadecimal 41 to 5A) are permitted;
     * lower-case characters shall not be used. The underscore (’_’,
     * decimal 95 or hexadecimal 5F) and hyphen (’-’, decimal 45
     * or hexadecimal 2D) are also permitted. No other characters
     * are permitted"
     *
     * [Appendix A]
     * keyword field := [keyword char...] [space...]
     */
    std::size_t nameLength = 0;
    bool spaceFound = false;

    for (const auto& c : keywordNameSpan)
    {
        if (c == ' ')
        {
            spaceFound = true;
        }
        else
        {
            // Can't be any characters after spaces have started (keyword must be left-aligned with no embedded spaces)
            if (spaceFound)
            {
                return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordName: Found embedded/leading spaces in keyword name"));
            }

            // Must be a valid character
            const bool isValidChar = IsDigit(c) || IsUpperCaseAlphabetic(c) || (c == '_') || (c == '-');
            if (!isValidChar)
            {
                return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordName: Invalid character in keyword name"));
            }

            nameLength++;
        }
    }

    if (nameLength == 0)
    {
        return std::nullopt;
    }

    return std::string(reinterpret_cast<const char*>(keywordNameSpan.data()), nameLength);
}

/**
 * Parses whether a keyword value contains a value indicator "= ". Note that this is different
 * from whether the keyword HAS a value, it's simply whether a value indicator exists or not.
 */
bool ParseValueIndicator(KeywordValueIndicatorCSpan valueIndicatorSpan)
{
    /**
     * [4.1.2.2.]
     * "If the two ASCII characters '= ' (decimal 61 followed
     * by decimal 32) are present in Bytes 9 and 10 of the keyword
     * record, this indicates that the keyword has a value field
     * associated with it, unless it is one of the commentary keywords
     * defined in Sect. 4.4.2 (i.e., a HISTORY, COMMENT, or completely
     * blank keyword name), which, by definition, have no value."
     */
    return valueIndicatorSpan[0] == '=' && valueIndicatorSpan[1] == ' ';
}

/**
 * Scans forwards through a keyword value span looking for the start of a comment, which is
 * defined by a forward slash that's not within a string. If a comment doesn't exist,
 * returns std::nullopt
 */
std::optional<std::size_t> FindCommentStartIndex(KeywordValueCSpan keywordValueSpan)
{
    /**
     * If a comment follows the value field, it must be preceded
     * by a slash (’/’, decimal 47 or hexadecimal 2F). A space be-
     * tween the value and the slash is strongly recommended. The
     * comment may contain any of the restricted set of ASCII-text
     * characters, decimal 32 through 126 (hexadecimal 20 through
     * 7E). The ASCII control characters with decimal values less than
     * 32 (including the null, tab, carriage return, and line-feed charac-
     * ters), and the delete character (decimal 127 or hexadecimal 7F)
     * must not appear anywhere within a keyword record.
     */

    bool inString = false;

    for (std::size_t pos = 0; pos < keywordValueSpan.size(); ++pos)
    {
        const auto& c = keywordValueSpan[pos];

        if (c == '\'')
        {
            bool isDoubleQuote = false;

            if (pos < (keywordValueSpan.size() - 1))
            {
                isDoubleQuote = keywordValueSpan[pos + 1] == '\'';
            }

            if (!isDoubleQuote)
            {
                inString = !inString;
            }
        }

        if (c == '/' && !inString)
        {
            return pos;
        }
    }
    
    // If we looked through the whole span and saw no comment, then there is none
    return std::nullopt;
}

std::expected<std::string, Error> ParseKeywordValue_AsDisplayString(KeywordValueCSpan keywordValueSpan, bool isFixedFormat)
{
    // Span which covers keywordValueSpan to the point where an (optional) comment starts
    std::span<const char> nonCommentSpan;

    /**
     * [4.2.3]
     * "If the value is a fixed-format integer, the ASCII representation
     * shall be right-justified in Bytes 11 through 30.
     */
    if (isFixedFormat)
    {
        nonCommentSpan = keywordValueSpan.subspan<0, 20>();
    }
    else
    {
        /**
         * [4.2.3]
         * "A free-format integer value follows the same rules as fixed-
         * format integers except that the ASCII representation may occur
         * anywhere within Bytes 11 through 80."
         */
        const auto commentStartIndex = FindCommentStartIndex(keywordValueSpan);

        // If there's no comment, use the whole span
        if (!commentStartIndex)
        {
            nonCommentSpan = keywordValueSpan;
        }
        // Otherwise, use the non-comment portion of the span
        else
        {
            nonCommentSpan = keywordValueSpan.subspan(0, *commentStartIndex);
        }
    }

    // Trim leading spaces from the span
    while (!nonCommentSpan.empty() && IsSpaceChar(nonCommentSpan.front()))
    {
        nonCommentSpan = nonCommentSpan.subspan(1);
    }

    // Trim trailing spaces from the span
    while (!nonCommentSpan.empty() && IsSpaceChar(nonCommentSpan.back()))
    {
        nonCommentSpan = nonCommentSpan.subspan(0, nonCommentSpan.size() - 1);
    }

    return std::string(nonCommentSpan.cbegin(), nonCommentSpan.cend());
}

/**
 * Parses an integer value out of a value span. Handles both fixed and free-floating integer values.
 *
 * The span is required to match the format: [space...] integer value [space...]
 *
 * [Appendix A]
 * integer value := [sign] digit [digit...]
 *   {Comment: Such an integer value is interpreted as a signed decimal number. It may contain leading zeros.}
 * sign := ‘-’ | ‘+’
 * digit := ‘0’–‘9’
 *
 * Any comment or non-value field must not be part of the span.
 */
std::expected<int64_t, Error> ParseValue_AsInteger(std::span<const char> valueSpan)
{
    std::string valStr;

    bool reachedNonSpaceChar = false;
    bool atLeastOneDigit = false;
    bool reachedStop = false;

    for (const auto& c : valueSpan)
    {
        const bool isDigitChar = IsDigit(c);
        const bool isSignChar = IsSignChar(c);
        const bool isValidChar = isDigitChar || isSignChar;
        const bool isSpaceChar = IsSpaceChar(c);

        // Skip over leading spaces before the value
        if (isSpaceChar && !reachedNonSpaceChar)
        {
            continue;
        }
        reachedNonSpaceChar = true;

        // Error if any non-space characters follow the value
        if (reachedStop && !isSpaceChar)
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsInteger: Non-space char detected following the value"));
        }

        // The sign character, if present, must come before any digits
        if (isSignChar && atLeastOneDigit)
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsInteger: Sign must come before any digits"));
        }

        if (isValidChar)
        {
            valStr.push_back(c);
        }
        else if (isSpaceChar)
        {
            reachedStop = true;
        }
        else
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsInteger: Encountered unexpected character"));
        }

        if (isDigitChar)
        {
            atLeastOneDigit = true;
        }
    }

    // An integer value requires at least one digit
    if (!atLeastOneDigit)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsInteger: Require at least one valid digit"));
    }

    try
    {
        return std::stol(valStr);
    }
    catch (const std::invalid_argument&)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsInteger: No conversion to integer could be performed: {}", valStr));
    }
    catch (const std::out_of_range&)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsInteger: Value out of range: {}", valStr));
    }
}

std::expected<int64_t, Error> ParseKeywordValue_AsInteger(KeywordValueCSpan keywordValueSpan, bool isFixedFormat)
{
    /**
     * [4.2.3]
     * "If the value is a fixed-format integer, the ASCII representation
     * shall be right-justified in Bytes 11 through 30.
     */
    if (isFixedFormat)
    {
        const auto valueSpan = keywordValueSpan.subspan<0, 20>();

        // Require right-justified
        if (valueSpan[valueSpan.size() - 1] == ' ')
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsInteger: Fixed format is not right-justified"));
        }

        return ParseValue_AsInteger(valueSpan);
    }

    /**
     * [4.2.3]
     * "A free-format integer value follows the same rules as fixed-
     * format integers except that the ASCII representation may occur
     * anywhere within Bytes 11 through 80."
     */
    const auto commentStartIndex = FindCommentStartIndex(keywordValueSpan);

    // If there's no comment, pass through the whole span to be parsed
    if (!commentStartIndex)
    {
        return ParseValue_AsInteger(keywordValueSpan);
    }
    // Otherwise, pass through the non-comment portion of the span to be parsed
    else
    {
        return ParseValue_AsInteger(keywordValueSpan.subspan(0, *commentStartIndex));
    }
}

/**
 * Parses a real value out of a value span. Handles both fixed and free-floating real values.
 *
 * The span is required to match the format: [space...] floating value [space...]
 *
 * [Appendix a]
 * floating value := decimal number [exponent]
 * decimal number := [sign] [integer part] [‘.’ [fraction part]]
 *   {Constraint: At least one of the integer part and fraction part must be present.}
 * integer part := digit | [digit...]
 * fraction part := digit | [digit...]
 * exponent := exponent letter [sign] digit [digit...]
 * exponent letter := ‘E’ | ‘D’
 *
 * Any comment or non-value field must not be part of the span.
 */
std::expected<double, Error> ParseValue_AsReal(std::span<const char> valueSpan)
{
    std::string valStr;

    bool reachedNonSpaceChar = false;
    bool atLeastOneDigit = false;
    bool atLeastOneDigitPostExponent = false;
    bool reachedExponent = false;
    bool reachedStop = false;

    for (const auto& c : valueSpan)
    {
        const bool isDigitChar = IsDigit(c);
        const bool isSignChar = IsSignChar(c);
        const bool isDecimalChar = IsDecimalPointChar(c);
        const bool isExponentChar = IsExponentChar(c);
        const bool isValidChar = isDigitChar || isSignChar || isDecimalChar || isExponentChar;
        const bool isSpaceChar = IsSpaceChar(c);

        // Skip over leading spaces before the value
        if (isSpaceChar && !reachedNonSpaceChar)
        {
            continue;
        }
        reachedNonSpaceChar = true;

        // Error if any non-space characters follow the value
        if (reachedStop && !isSpaceChar)
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Non-space char detected following the value"));
        }

        // The sign character, if present, must come before any digits
        if (isSignChar)
        {
            if (!reachedExponent && atLeastOneDigit)
            {
                return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Sign must come before any digits"));
            }
            else if (reachedExponent && atLeastOneDigitPostExponent)
            {
                return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Sign must come before any digits in exponent"));
            }
        }

        // The decimal character, if present, must come after at least one digit
        if (isDecimalChar && !atLeastOneDigit)
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Decimal must come after one or more digits"));
        }

        if (isExponentChar)
        {
            // The exponent character, if present, must come after at least one digit
            if (!atLeastOneDigit)
            {
                return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Exponent must come after one or more digits"));
            }

            reachedExponent = true;
        }

        if (isValidChar)
        {
            valStr.push_back(c);
        }
        else if (isSpaceChar)
        {
            reachedStop = true;
        }
        else
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Encountered unexpected character"));
        }

        if (isDigitChar)
        {
            atLeastOneDigit = true;

            if (reachedExponent)
            {
                atLeastOneDigitPostExponent = true;
            }
        }
    }

    // A real value requires at least one digit
    if (!atLeastOneDigit)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Require at least one valid digit"));
    }

    // If exponent is present, requires at least one digit
    if (reachedExponent && !atLeastOneDigitPostExponent)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Require at least one valid digit following an exponent"));
    }

    try
    {
        return std::stod(valStr);
    }
    catch (const std::invalid_argument&)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: No conversion to double could be performed: {}", valStr));
    }
    catch (const std::out_of_range&)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseValue_AsReal: Value out of range: {}", valStr));
    }
}

std::expected<double, Error> ParseKeywordValue_AsReal(KeywordValueCSpan keywordValueSpan, bool isFixedFormat)
{
    /**
     * [4.2.4]
     * If the value is a fixed-format real floating-point number, the
     * ASCII representation shall be right-justified in Bytes 11 through
     * 30.
     */
    if (isFixedFormat)
    {
        const auto valueSpan = keywordValueSpan.subspan<0, 20>();

        // Require right-justified
        if (valueSpan[valueSpan.size() - 1] == ' ')
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsReal: Fixed format is not right-justified"));
        }

        return ParseValue_AsReal(valueSpan);
    }

    /**
     * [4.2..4]
     * A free-format floating-point value follows the same rules as
     * a fixed-format floating-point value except that the ASCII repre-
     * sentation may occur anywhere within Bytes 11 through 80.
     */
    const auto commentStartIndex = FindCommentStartIndex(keywordValueSpan);

    // If there's no comment, pass through the whole span to be parsed
    if (!commentStartIndex)
    {
        return ParseValue_AsReal(keywordValueSpan);
    }
    // Otherwise, pass through the non-comment portion of the span to be parsed
    else
    {
        return ParseValue_AsReal(keywordValueSpan.subspan(0, *commentStartIndex));
    }
}

}
