/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseKeywordValueAsInteger, FixedFormat_HappyPath)
{
    // Setup
    const std::string valueField = "                   2";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 2);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_MultipleDigits)
{
    // Setup
    const std::string valueField = "                 123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingPlusSign)
{
    // Setup
    const std::string valueField = "                +123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingNegativeSign)
{
    // Setup
    const std::string valueField = "                -123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, -123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingZeroes)
{
    // Setup
    const std::string valueField = "               00123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingZeroesWithSign)
{
    // Setup
    const std::string valueField = "              +00123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_RequireRightJustified)
{
    // Setup
    const std::string valueField = "             +00123 ";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_BadCharStart)
{
    // Setup
    const std::string valueField = "             a+00123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_BadCharEnd)
{
    // Setup
    const std::string valueField = "             +00123a";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_EmbeddedSpace)
{
    // Setup
    const std::string valueField = "             +00 123";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_NoDigits)
{
    // Setup
    const std::string valueField = "                    ";
    ASSERT_EQ(valueField.length(), 20);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FreeFormat_HappyPath)
{
    // Setup
    const std::string valueField = "                 123  / FLIGHT22 05Apr96 RSH                          ";
    ASSERT_EQ(valueField.length(), 70);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FreeFormat_NoComment)
{
    // Setup
    const std::string valueField = "                 123                                                  ";
    ASSERT_EQ(valueField.length(), 70);

    const KeywordValueCSpan valueSpan = KeywordValueCSpan(valueField);

    // Act
    const auto result = ParseKeywordValue_AsInteger(valueSpan, false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}
