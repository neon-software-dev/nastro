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
    const std::string keywordRecord = "KEYWORD =                    2                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 2);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_MultipleDigits)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                  123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingPlusSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                 +123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingNegativeSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                 -123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, -123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingZeroes)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                00123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_LeadingZeroesWithSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =               +00123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_RequireRightJustified)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =              +00123                                                   ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_BadCharStart)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =              a+00123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_BadCharEnd)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =              +00123a                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_EmbeddedSpace)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =              +00 123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FixedFormat_NoDigits)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                                                                       ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordValueAsInteger, FreeFormat_WithComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                            123  / FLIGHT22 05Apr96 RSH                ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}

TEST(ParseKeywordValueAsInteger, FreeFormat_NoComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                            123                                        ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsInteger(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 123);
}
