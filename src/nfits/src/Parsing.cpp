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
 * Scans forwards through a keyword record looking for the start of a comment, which is
 * defined by a forward slash that's not within a string. If a comment doesn't exist,
 * returns std::nullopt
 */
std::optional<std::size_t> FindCommentStartIndex(KeywordRecordCSpan keywordRecordSpan)
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

    // Note: Starting at 10, to skip over keyword and value indicator
    for (std::size_t pos = 10; pos < keywordRecordSpan.size(); ++pos)
    {
        const auto& c = keywordRecordSpan[pos];

        if (c == '\'')
        {
            bool isDoubleQuote = false;

            if (pos < (keywordRecordSpan.size() - 1))
            {
                isDoubleQuote = keywordRecordSpan[pos + 1] == '\'';
            }

            if (!isDoubleQuote)
            {
                inString = !inString;
            }
        }

        // A forward slash not in a string denotes the start of a comment
        if (c == '/' && !inString)
        {
            return pos;
        }
    }
    
    // No comment was found
    return std::nullopt;
}

/**
 * Parses an integer value out of a value span. Handles both fixed and free-floating integer values.
 *
 * The span is required to match the format: [space...] integer value [space...]
 *
 * Note that any comment should not be included in valueSpan.
 *
 * [Appendix A]
 * integer value := [sign] digit [digit...]
 *   {Comment: Such an integer value is interpreted as a signed decimal number. It may contain leading zeros.}
 * sign := ‘-’ | ‘+’
 * digit := ‘0’–‘9’
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

std::expected<int64_t, Error> ParseKeywordValue_AsInteger(KeywordRecordCSpan keywordRecordSpan, bool isFixedFormat)
{
    /**
     * [4.2.3]
     * "If the value is a fixed-format integer, the ASCII representation
     * shall be right-justified in Bytes 11 through 30.
     */
    if (isFixedFormat)
    {
        const auto valueSpan = keywordRecordSpan.subspan<10, 20>();

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
    const auto commentStartIndex = FindCommentStartIndex(keywordRecordSpan);

    // If there's no comment, pass through the whole span to be parsed
    if (!commentStartIndex)
    {
        return ParseValue_AsInteger(keywordRecordSpan.subspan(10));
    }
    // Otherwise, pass through the non-comment portion of the span to be parsed
    else
    {
        return ParseValue_AsInteger(keywordRecordSpan.subspan(10, *commentStartIndex - 10));
    }
}

/**
 * Parses a real value out of a value span. Handles both fixed and free-floating real values.
 *
 * The span is required to match the format: [space...] floating value [space...]
 *
 * Note that any comment should not be included in valueSpan.
 *
 * [Appendix a]
 * floating value := decimal number [exponent]
 * decimal number := [sign] [integer part] [‘.’ [fraction part]]
 *   {Constraint: At least one of the integer part and fraction part must be present.}
 * integer part := digit | [digit...]
 * fraction part := digit | [digit...]
 * exponent := exponent letter [sign] digit [digit...]
 * exponent letter := ‘E’ | ‘D’
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

std::expected<double, Error> ParseKeywordValue_AsReal(KeywordRecordCSpan keywordRecordSpan, bool isFixedFormat)
{
    /**
     * [4.2.4]
     * If the value is a fixed-format real floating-point number, the
     * ASCII representation shall be right-justified in Bytes 11 through
     * 30.
     */
    if (isFixedFormat)
    {
        const auto valueSpan = keywordRecordSpan.subspan<10, 20>();

        // Require right-justified; no space at the end
        if (IsSpaceChar(valueSpan.last(1)[0]))
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsReal: Fixed format is not right-justified"));
        }

        return ParseValue_AsReal(valueSpan);
    }

    /**
     * [4.2.4.]
     * A free-format floating-point value follows the same rules as
     * a fixed-format floating-point value except that the ASCII repre-
     * sentation may occur anywhere within Bytes 11 through 80.
     */
    const auto commentStartIndex = FindCommentStartIndex(keywordRecordSpan);

    // If there's no comment, pass through the whole value span to be parsed
    if (!commentStartIndex)
    {
        return ParseValue_AsReal(keywordRecordSpan.subspan(10));
    }
    // Otherwise, pass through the non-comment portion of the value span to be parsed
    else
    {
        return ParseValue_AsReal(keywordRecordSpan.subspan(10, *commentStartIndex - 10));
    }
}

std::expected<bool, Error> ParseKeywordValue_AsLogical(KeywordRecordCSpan keywordRecordSpan, bool isFixedFormat)
{
    std::optional<char> logicalChar;

    /**
     * [4.2.2.]
     * If the value is a fixed-format logical constant, it shall appear as
     * an upper-case T or F in Byte 30.
     */
    if (isFixedFormat)
    {
        logicalChar = keywordRecordSpan[29];
    }
    /**
    * [4.2.2.]
    * A logical value is represented
    * in free-format by a single character consisting of an upper-case
    * T or F as the first non-space character in Bytes 11 through 80.
    */
    else
    {
        const auto valueSpan = keywordRecordSpan.subspan(10, 70);

        const auto it = std::ranges::find_if(valueSpan, [](const char& c) { return !IsSpaceChar(c); });
        if (it != valueSpan.cend())
        {
            logicalChar = *it;
        }
    }

    if (!logicalChar)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsLogical: No logical value character detected"));
    }
    else if (*logicalChar == 'T')
    {
        return true;
    }
    else if (*logicalChar == 'F')
    {
        return false;
    }
    else
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsLogical: Invalid logical character: {}", *logicalChar));
    }
}

std::expected<std::string, Error> ParseKeywordValue_AsString(KeywordRecordCSpan keywordRecordSpan, bool isFixedFormat)
{
    const auto commentStartIndex = FindCommentStartIndex(keywordRecordSpan);

    // When we're searching the string we stop where the comment starts, if it exists, or else the end of the record, if not
    const auto stringSearchEndPos = commentStartIndex ? *commentStartIndex : keywordRecordSpan.size();

    //
    // Find the string starting quote
    //
    std::size_t startQuotePos = 0;

    if (isFixedFormat)
    {
        /**
         * [4.2.1]
         * If the value is a fixed-format character string, the starting
         * single-quote character must be in Byte 11 of the keyword record
         */
        if (keywordRecordSpan[10] != '\'')
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsString: Fixed format string doesn't have starting quote in pos 10"));
        }

        startQuotePos = 10;
    }
    else
    {
        /**
         * [4.2.1]
         * Free-format character strings follow the same rules as fixed-
         * format character strings except that the starting single-quote
         * character may occur after Byte 11. Any bytes preceding the start-
         * ing quote character and after Byte 10 must contain the space
         * character.
         */
        bool foundStartQuotePos = false;

        for (std::size_t pos = 10; pos < stringSearchEndPos; ++pos)
        {
            const char& c = keywordRecordSpan[pos];

            if (c == '\'')
            {
                startQuotePos = pos;
                foundStartQuotePos = true;
                break;
            }
            else if (!IsSpaceChar(c))
            {
                return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsString: Non-space character found before start quote"));
            }
        }

        if (!foundStartQuotePos)
        {
            return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsString: Free format string, no start quote found"));
        }
    }

    //
    // Find the string ending quote
    //
    std::size_t endQuotePos = 0;

    /**
     * [4.2.1]
     * [..] A single quote is represented within a string as two successive single quotes
     * [..] the closing single quote must occur in or before Byte 80
     */
    bool foundEndQuotePos = false;

    for (std::size_t pos = startQuotePos + 1; pos < stringSearchEndPos; ++pos)
    {
        const char& c = keywordRecordSpan[pos];

        if (c == '\'')
        {
            // If the quote is the last character, then it can't be an escaped single quote, so it's the closing quote
            if (pos == stringSearchEndPos - 1)
            {
                endQuotePos = pos;
                foundEndQuotePos = true;
                break;
            }

            // If the next character is also a quote, then this is an escaped quote definition, so skip forward past it
            if (keywordRecordSpan[pos+1] == '\'')
            {
                pos++;
            }
            // Otherwise, if the next character isn't a quote, defining an escaped quote, then we're looking at a closing quote
            else
            {
                endQuotePos = pos;
                foundEndQuotePos = true;
                break;
            }
        }
    }

    if (!foundEndQuotePos)
    {
        return std::unexpected(Error::Msg(ErrorType::Parse, "ParseKeywordValue_AsString: Failed to find closing quote"));
    }

    //
    // Convert the string from an escaped string to a non-escaped string
    //
    const auto stringSpan = keywordRecordSpan.subspan(startQuotePos + 1, endQuotePos - startQuotePos - 1);

    std::string stringVal;

    for (std::size_t pos = 0; pos < stringSpan.size(); ++pos)
    {
        const char& c = stringSpan[pos];

        stringVal += c;

        // If the char is a quote, add it to the string, but then skip over the next character, which would be another quote
        if (c == '\'')
        {
            pos++;
        }
    }

    //
    // Strip trailing whitespace from the string
    //

    /**
     * [4.2.1]
     * Leading spaces are significant; trailing spaces are not.
     */
    while (stringVal.length() > 1 && IsSpaceChar(stringVal.at(stringVal.size() - 1)))
    {
        stringVal = stringVal.substr(0, stringVal.size() - 1);
    }

    return stringVal;
}

}
