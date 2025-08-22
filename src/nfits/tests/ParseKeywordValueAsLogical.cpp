/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseKeywordValueAsLogical, FixedFormat_T_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                    T                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, true);
}

TEST(ParseKeywordValueAsLogical, FixedFormat_F_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                    F                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, false);
}

TEST(ParseKeywordValueAsLogical, FixedFormat_BadChar)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                    X                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsLogical, FixedFormat_WrongPosChar)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =               T                                                       ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsLogical, FixedFormat_WithComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                    T                 / ABCD EFGH                      ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), true);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, true);
}

TEST(ParseKeywordValueAsLogical, FreeFormat_T_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                              T                                        ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, true);
}

TEST(ParseKeywordValueAsLogical, FreeFormat_F_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                              F                                        ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, false);
}

TEST(ParseKeywordValueAsLogical, FreeFormat_WithComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                              T      / ABCD EFGH                       ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, true);
}

TEST(ParseKeywordValueAsLogical, FreeFormat_MoreThanOneChar)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                              T         F                              ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, true);
}

TEST(ParseKeywordValueAsLogical, FreeFormat_BadCharBeforeLogicalChar)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =              x               T                                        ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsLogical, FreeFormat_WrongDataType)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                             'T'                                       ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsLogical(KeywordRecordCSpan(keywordRecord), false);

    // Assert
    ASSERT_FALSE(result);
}
