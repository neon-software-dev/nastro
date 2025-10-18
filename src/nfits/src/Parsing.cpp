/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "Parsing.h"

#include <algorithm>
#include <queue>

namespace NFITS
{

static constexpr auto IsUpperCaseAlphabetic = [](const char& c){ return 'A' <= c && c <= 'Z'; };
static constexpr auto IsDigit = [](const char& c){ return '0' <= c && c <= '9'; };
static constexpr auto IsASCIITextChar = [](const char& c){ return 32 <= c && c <= 126; };
static constexpr auto IsSpaceChar = [](const char& c){ return c == ' '; };
static constexpr auto IsSignChar = [](const char& c){ return (c == '+') || (c == '-'); };
static constexpr auto IsDecimalPointChar = [](const char& c){ return c == '.'; };
static constexpr auto IsExponentChar = [](const char& c){ return (c == 'E') || (c == 'D'); };

std::expected<int, Error> ToInt(const std::string& str)
{
    int i = 0;
    try
    {
        i = std::stoi(str);
    }
    catch (const std::invalid_argument&)
    {
        return std::unexpected(Error::Msg("No conversion to integer could be performed: {}", str));
    }
    catch (const std::out_of_range&)
    {
        return std::unexpected(Error::Msg("Value out of range: {}", str));
    }

    return i;
}

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
                return std::unexpected(Error::Msg("ParseKeywordName: Found embedded/leading spaces in keyword name"));
            }

            // Must be a valid character
            const bool isValidChar = IsDigit(c) || IsUpperCaseAlphabetic(c) || (c == '_') || (c == '-');
            if (!isValidChar)
            {
                return std::unexpected(Error::Msg("ParseKeywordName: Invalid character in keyword name"));
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
            return std::unexpected(Error::Msg("ParseValue_AsInteger: Non-space char detected following the value"));
        }

        // The sign character, if present, must come before any digits
        if (isSignChar && atLeastOneDigit)
        {
            return std::unexpected(Error::Msg("ParseValue_AsInteger: Sign must come before any digits"));
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
            return std::unexpected(Error::Msg("ParseValue_AsInteger: Encountered unexpected character"));
        }

        if (isDigitChar)
        {
            atLeastOneDigit = true;
        }
    }

    // An integer value requires at least one digit
    if (!atLeastOneDigit)
    {
        return std::unexpected(Error::Msg("ParseValue_AsInteger: Require at least one valid digit"));
    }

    try
    {
        return std::stol(valStr);
    }
    catch (const std::invalid_argument&)
    {
        return std::unexpected(Error::Msg("ParseValue_AsInteger: No conversion to integer could be performed: {}", valStr));
    }
    catch (const std::out_of_range&)
    {
        return std::unexpected(Error::Msg("ParseValue_AsInteger: Value out of range: {}", valStr));
    }
}

std::expected<int64_t, Error> ParseKeywordValue_AsInteger(KeywordRecordCSpan keywordRecordSpan)
{
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
            return std::unexpected(Error::Msg("ParseValue_AsReal: Non-space char detected following the value"));
        }

        // The sign character, if present, must come before any digits
        if (isSignChar)
        {
            if (!reachedExponent && atLeastOneDigit)
            {
                return std::unexpected(Error::Msg("ParseValue_AsReal: Sign must come before any digits"));
            }
            else if (reachedExponent && atLeastOneDigitPostExponent)
            {
                return std::unexpected(Error::Msg("ParseValue_AsReal: Sign must come before any digits in exponent"));
            }
        }

        // The decimal character, if present, must come after at least one digit
        if (isDecimalChar && !atLeastOneDigit)
        {
            return std::unexpected(Error::Msg("ParseValue_AsReal: Decimal must come after one or more digits"));
        }

        if (isExponentChar)
        {
            // The exponent character, if present, must come after at least one digit
            if (!atLeastOneDigit)
            {
                return std::unexpected(Error::Msg("ParseValue_AsReal: Exponent must come after one or more digits"));
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
            return std::unexpected(Error::Msg("ParseValue_AsReal: Encountered unexpected character"));
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
        return std::unexpected(Error::Msg("ParseValue_AsReal: Require at least one valid digit"));
    }

    // If exponent is present, requires at least one digit
    if (reachedExponent && !atLeastOneDigitPostExponent)
    {
        return std::unexpected(Error::Msg("ParseValue_AsReal: Require at least one valid digit following an exponent"));
    }

    try
    {
        return std::stod(valStr);
    }
    catch (const std::invalid_argument&)
    {
        return std::unexpected(Error::Msg("ParseValue_AsReal: No conversion to double could be performed: {}", valStr));
    }
    catch (const std::out_of_range&)
    {
        return std::unexpected(Error::Msg("ParseValue_AsReal: Value out of range: {}", valStr));
    }
}

std::expected<double, Error> ParseKeywordValue_AsReal(KeywordRecordCSpan keywordRecordSpan)
{
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

std::expected<bool, Error> ParseKeywordValue_AsLogical(KeywordRecordCSpan keywordRecordSpan)
{
    std::optional<char> logicalChar;

    /**
    * [4.2.2.]
    * A logical value is represented
    * in free-format by a single character consisting of an upper-case
    * T or F as the first non-space character in Bytes 11 through 80.
    */
    const auto valueSpan = keywordRecordSpan.subspan(10, 70);

    const auto it = std::ranges::find_if(valueSpan, [](const char& c) { return !IsSpaceChar(c); });
    if (it != valueSpan.end())
    {
        logicalChar = *it;
    }

    if (!logicalChar)
    {
        return std::unexpected(Error::Msg("ParseKeywordValue_AsLogical: No logical value character detected"));
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
        return std::unexpected(Error::Msg("ParseKeywordValue_AsLogical: Invalid logical character: {}", *logicalChar));
    }
}

std::expected<std::string, Error> ParseKeywordValue_AsString(KeywordRecordCSpan keywordRecordSpan)
{
    const auto commentStartIndex = FindCommentStartIndex(keywordRecordSpan);

    // When we're searching the string we stop where the comment starts, if it exists, or else the end of the record, if not
    const auto stringSearchEndPos = commentStartIndex ? *commentStartIndex : keywordRecordSpan.size();

    //
    // Find the string starting quote
    //
    std::size_t startQuotePos = 0;

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
            return std::unexpected(Error::Msg("ParseKeywordValue_AsString: Non-space character found before start quote"));
        }
    }

    if (!foundStartQuotePos)
    {
        return std::unexpected(Error::Msg("ParseKeywordValue_AsString: Free format string, no start quote found"));
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
        return std::unexpected(Error::Msg("ParseKeywordValue_AsString: Failed to find closing quote"));
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

std::expected<BinFieldType, Error> GetBinFieldTypeFromChar(char typeChar)
{
    switch (typeChar)
    {
        case 'L': return BinFieldType::Logical;
        case 'X': return BinFieldType::Bit;
        case 'B': return BinFieldType::UnsignedByte;
        case 'I': return BinFieldType::Integer16Bit;
        case 'J': return BinFieldType::Integer32Bit;
        case 'K': return BinFieldType::Integer64Bit;
        case 'A': return BinFieldType::Character;
        case 'E': return BinFieldType::FloatSinglePrecision;
        case 'D': return BinFieldType::FloatDoublePrecision;
        case 'C': return BinFieldType::ComplexSinglePrecision;
        case 'M': return BinFieldType::ComplexDoublePrecision;
        case 'P': return BinFieldType::Array32Bit;
        case 'Q': return BinFieldType::Array64Bit;
        default: return std::unexpected(Error::Msg("Invalid type character: {}", typeChar));
    }
}

std::expected<BinFieldForm, Error> ParseBinTable_TFORMN(const std::string& tformn)
{
    BinFieldForm fieldForm{};

    if (tformn.empty())
    {
        return std::unexpected(Error::Msg("tformn can not be empty"));
    }

    // Find the first non-numeric digit, to find the delimitation between repeat count and type
    const auto nonDigitIt = std::ranges::find_if(tformn, [](const char& c){
        return !IsDigit(c);
    });
    if (nonDigitIt == tformn.cend())
    {
        return std::unexpected(Error::Msg("tformn doesn't contain type specifier"));
    }

    //
    // Parse repeat count, if present
    //
    if (nonDigitIt != tformn.cbegin())
    {
        const auto repeatCountSubstring = std::string(tformn.cbegin(), nonDigitIt);
        const auto repeatCount = ToInt(repeatCountSubstring);
        if (!repeatCount)
        {
            return std::unexpected(repeatCount.error());
        }

        if (*repeatCount < 0)
        {
            return std::unexpected(Error::Msg("Repeat count must be non-negative: {}", *repeatCount));
        }

        fieldForm.repeatCount = static_cast<uintmax_t>(*repeatCount);
    }

    //
    // Parse field type
    //
    const auto typeCharIndex = static_cast<std::size_t>(std::distance(tformn.cbegin(), nonDigitIt));
    const auto typeChar = tformn.at(typeCharIndex);
    const auto fieldType = GetBinFieldTypeFromChar(typeChar);
    if (!fieldType)
    {
        return std::unexpected(fieldType.error());
    }
    fieldForm.type = *fieldType;

    //
    // Parse (optional) array field type
    //
    std::size_t arrayTypeCharIndex = 0;
    if (fieldForm.type == BinFieldType::Array32Bit || fieldForm.type == BinFieldType::Array64Bit)
    {
        arrayTypeCharIndex = typeCharIndex + 1;

        // Require the tformn to have more characters when an array is involved
        if (tformn.length() <= arrayTypeCharIndex)
        {
            return std::unexpected(Error::Msg("Array field type character must be present"));
        }

        const auto arrayTypeChar = tformn.at(arrayTypeCharIndex);
        const auto arrayFieldType = GetBinFieldTypeFromChar(arrayTypeChar);
        if (!arrayFieldType)
        {
            return std::unexpected(arrayFieldType.error());
        }

        // The array field type can't be an array field type
        if (*arrayFieldType == BinFieldType::Array32Bit || *arrayFieldType == BinFieldType::Array64Bit)
        {
            return std::unexpected(Error::Msg("Invalid array field type character: {}", arrayTypeChar));
        }

        fieldForm.arrayType = *arrayFieldType;
    }

    //
    // Parse (optional) array max count
    //
    if (fieldForm.arrayType)
    {
        const auto arrayMaxCountStartIt = std::ranges::find_if(tformn, [](const char c){
            return c == '(';
        });
        if (arrayMaxCountStartIt == tformn.cend())
        {
            return std::unexpected(Error::Msg("Array field missing max element count start char"));
        }

        const auto arrayMaxCountCloseIt = std::ranges::find_if(tformn, [](const char c){
            return c == ')';
        });
        if (arrayMaxCountCloseIt == tformn.cend())
        {
            return std::unexpected(Error::Msg("Array field missing max element count close char"));
        }

        const auto arrayMaxCountStartIndex = static_cast<std::size_t>(std::distance(tformn.cbegin(), arrayMaxCountStartIt));
        const auto arrayMaxCountCloseIndex = static_cast<std::size_t>(std::distance(tformn.cbegin(), arrayMaxCountCloseIt));

        // Max count opening '(' should immediately follow array type character
        if (arrayMaxCountStartIndex != arrayTypeCharIndex + 1)
        {
            return std::unexpected(Error::Msg("Invalid array max count specifier"));
        }

        // Should be characters inbetween ( and )
        if (arrayMaxCountStartIndex + 1 == arrayMaxCountCloseIndex)
        {
            return std::unexpected(Error::Msg("Invalid array max count specifier"));
        }

        const auto arrayMaxCountString = tformn.substr(arrayMaxCountStartIndex + 1, arrayMaxCountCloseIndex - arrayMaxCountStartIndex - 1);
        const auto arrayMaxCount = ToInt(arrayMaxCountString);
        if (!arrayMaxCount)
        {
            return std::unexpected(arrayMaxCount.error());
        }

        if (*arrayMaxCount < 0)
        {
            return std::unexpected(Error::Msg("Array max count count must be non-negative: {}", *arrayMaxCount));
        }

        fieldForm.arrayMaxCount = static_cast<uintmax_t>(*arrayMaxCount);
    }

    return fieldForm;
}

std::optional<WCSKeywordName> ParseWCSKeywordName_a(const std::string& keywordName, const std::string& baseName)
{
    WCSKeywordName wcsKeywordName{};

    // If keyword doesn't start with basename, bail out
    if (!keywordName.starts_with(baseName))
    {
        return std::nullopt;
    }

    wcsKeywordName.name = keywordName;
    wcsKeywordName.base = baseName;

    // Basename matches exactly; no 'a'
    if (keywordName == baseName)
    {
        return wcsKeywordName;
    }
    // Otherwise, if there's one more char, it's the 'a'
    else if (keywordName.length() == baseName.length() + 1)
    {
        wcsKeywordName.a = keywordName.at(baseName.length());

        // Validate that the 'a' is actually A-Z
        if (!IsUpperCaseAlphabetic(*wcsKeywordName.a))
        {
            return std::nullopt;
        }

        return wcsKeywordName;
    }

    // If keywordName has more than one extra char, invalid format
    return std::nullopt;
}

std::optional<WCSKeywordName> ParseWCSKeywordName_ia(const std::string& keywordName, const std::string& baseName)
{
    WCSKeywordName wcsKeywordName{};

    // If keyword doesn't start with basename, bail out
    if (!keywordName.starts_with(baseName))
    {
        return std::nullopt;
    }

    wcsKeywordName.name = keywordName;
    wcsKeywordName.base = baseName;

    // Basename matches exactly - missing 'i', invalid
    if (keywordName == baseName)
    {
        return std::nullopt;
    }

    std::queue<char> q;

    for (std::size_t x = baseName.length(); x < keywordName.length(); ++x)
    {
        q.push(keywordName.at(x));
    }

    // Parse 'i' - A series of integer characters
    std::string iStr;

    while (!q.empty() && IsDigit(q.front()))
    {
        iStr += q.front();
        q.pop();
    }

    // Validate 'i'
    if (iStr.empty())
    {
        return std::nullopt;
    }

    const auto i = ToInt(iStr);
    if (!i)
    {
        return std::nullopt;
    }

    wcsKeywordName.i = static_cast<int64_t>(*i);

    // Parse optional 'a'
    if (!q.empty() && IsUpperCaseAlphabetic(q.front()))
    {
        wcsKeywordName.a = q.front();
        q.pop();
    }

    // If there's anything left over, unprocessed, bad keyword name
    if (!q.empty())
    {
        return std::nullopt;
    }

    return wcsKeywordName;
}

std::optional<WCSKeywordName> ParseWCSKeywordName_ja(const std::string& keywordName, const std::string& baseName)
{
    WCSKeywordName wcsKeywordName{};

    // If keyword doesn't start with basename, bail out
    if (!keywordName.starts_with(baseName))
    {
        return std::nullopt;
    }

    wcsKeywordName.name = keywordName;
    wcsKeywordName.base = baseName;

    // Basename matches exactly - missing 'j', invalid
    if (keywordName == baseName)
    {
        return std::nullopt;
    }

    std::queue<char> q;

    for (std::size_t x = baseName.length(); x < keywordName.length(); ++x)
    {
        q.push(keywordName.at(x));
    }

    // Parse 'j' - A series of integer characters
    std::string jStr;

    while (!q.empty() && IsDigit(q.front()))
    {
        jStr += q.front();
        q.pop();
    }

    // Validate 'j'
    if (jStr.empty())
    {
        return std::nullopt;
    }

    const auto j = ToInt(jStr);
    if (!j)
    {
        return std::nullopt;
    }

    wcsKeywordName.j = static_cast<int64_t>(*j);

    // Parse optional 'a'
    if (!q.empty() && IsUpperCaseAlphabetic(q.front()))
    {
        wcsKeywordName.a = q.front();
        q.pop();
    }

    // If there's anything left over, unprocessed, bad keyword name
    if (!q.empty())
    {
        return std::nullopt;
    }

    return wcsKeywordName;
}

std::optional<WCSKeywordName> ParseWCSKeywordName_i(const std::string& keywordName, const std::string& baseName)
{
    WCSKeywordName wcsKeywordName{};

    // If keyword doesn't start with basename, bail out
    if (!keywordName.starts_with(baseName))
    {
        return std::nullopt;
    }

    wcsKeywordName.name = keywordName;
    wcsKeywordName.base = baseName;

    // Basename matches exactly - missing 'i', invalid
    if (keywordName == baseName)
    {
        return std::nullopt;
    }

    std::queue<char> q;

    for (std::size_t x = baseName.length(); x < keywordName.length(); ++x)
    {
        q.push(keywordName.at(x));
    }

    // Parse 'i' - A series of integer characters
    std::string iStr;

    while (!q.empty() && IsDigit(q.front()))
    {
        iStr += q.front();
        q.pop();
    }

    // Validate 'i'
    if (iStr.empty())
    {
        return std::nullopt;
    }

    const auto i = ToInt(iStr);
    if (!i)
    {
        return std::nullopt;
    }

    wcsKeywordName.i = static_cast<int64_t>(*i);

    // If there's anything left over, unprocessed, bad keyword name
    if (!q.empty())
    {
        return std::nullopt;
    }

    return wcsKeywordName;
}

std::optional<WCSKeywordName> ParseWCSKeywordName_i_ja(const std::string& keywordName, const std::string& baseName)
{
    WCSKeywordName wcsKeywordName{};

    // If keyword doesn't start with basename, bail out
    if (!keywordName.starts_with(baseName))
    {
        return std::nullopt;
    }

    wcsKeywordName.name = keywordName;
    wcsKeywordName.base = baseName;

    // Basename matches exactly - missing 'i', invalid
    if (keywordName == baseName)
    {
        return std::nullopt;
    }

    std::queue<char> q;

    for (std::size_t x = baseName.length(); x < keywordName.length(); ++x)
    {
        q.push(keywordName.at(x));
    }

    // Parse 'i' - A series of integer characters
    std::string iStr;

    while (!q.empty() && IsDigit(q.front()))
    {
        iStr += q.front();
        q.pop();
    }

    // Validate 'i'
    if (iStr.empty())
    {
        return std::nullopt;
    }

    const auto i = ToInt(iStr);
    if (!i)
    {
        return std::nullopt;
    }

    wcsKeywordName.i = static_cast<int64_t>(*i);

    // Next character must be underscore separating i and j
    if (q.empty() || q.front() != '_')
    {
        return std::nullopt;
    }

    q.pop();

    // Parse 'j' - A series of integer characters
    std::string jStr;

    while (!q.empty() && IsDigit(q.front()))
    {
        jStr += q.front();
        q.pop();
    }

    // Validate 'j'
    if (jStr.empty())
    {
        return std::nullopt;
    }

    const auto j = ToInt(jStr);
    if (!j)
    {
        return std::nullopt;
    }

    wcsKeywordName.j = static_cast<int64_t>(*j);

    // Parse optional 'a'
    if (!q.empty() && IsUpperCaseAlphabetic(q.front()))
    {
        wcsKeywordName.a = q.front();
        q.pop();
    }

    // If there's anything left over, unprocessed, bad keyword name
    if (!q.empty())
    {
        return std::nullopt;
    }

    return wcsKeywordName;
}

std::expected<WCSCType, Error> ParseWCSCType(const std::string& ctype)
{
    /**
     * [8.2]
     * [..] the first four characters specify the coordinate
     * type, the fifth character is a hyphen (‘-’), and the remain-
     * ing three characters specify an algorithm code for computing
     * the world coordinate value. Coordinate types with names of
     * fewer than four characters are padded on the right with hy-
     * phens, and algorithm codes with fewer than three charac-
     * ters are padded on the right with blanks
     */

    //
    // If not 8 chars with a hyphen at fifth character, consider the whole string a linear coordinate type
    //
    if (ctype.length() != 8) { return WCSLinearCType{.coordinateType = ctype}; }
    if (ctype.at(4) != '-') { return WCSLinearCType{.coordinateType = ctype}; }

    //
    // Otherwise, build a non-linear ctype
    //
    WCSNonLinearCType nonLinearType{};

    std::queue<char> q;
    for (const auto& c : ctype)
    {
        q.push(c);
    }

    // Read four characters, padded on right by hyphens
    bool inTypePadding = false;

    for (std::size_t x = 0; x < 4; ++x)
    {
        if (IsUpperCaseAlphabetic(q.front()) && !inTypePadding)
        {
            nonLinearType.coordinateType.push_back(q.front());
            q.pop();
        }
        else if (q.front() == '-')
        {
            inTypePadding = true;
            q.pop();
        }
        else
        {
            return std::unexpected(Error::Msg("Invalid non-linear coordinate type chars"));
        }
    }

    // Pop the dividing hyphen
    q.pop();

    // Read three characters, padded on right by spaces
    bool inCodePadding = false;

    for (std::size_t x = 0; x < 3; ++x)
    {
        if (IsUpperCaseAlphabetic(q.front()) && !inCodePadding)
        {
            nonLinearType.algorithmCode.push_back(q.front());
            q.pop();
        }
        else if (q.front() == ' ')
        {
            inCodePadding = true;
            q.pop();
        }
        else
        {
            return std::unexpected(Error::Msg("Invalid non-linear algorithm code chars"));
        }
    }

    return nonLinearType;
}

}
