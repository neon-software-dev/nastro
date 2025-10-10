/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseKeywordValueAsString, FixedFormat_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 'ABCD'                                                                ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD");
}

TEST(ParseKeywordValueAsString, FixedFormat_WithComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 'ABCD'               / COMMENT HERE                                   ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD");
}

TEST(ParseKeywordValueAsString, FixedFormat_EscapedChar)
{
    // Setup
    const std::string keywordRecord = R"(KEYWORD = 'AB''CD'               / COMMENT HERE                                 )";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, R"(AB'CD)");
}

TEST(ParseKeywordValueAsString, FixedFormat_EscapedCharAtEnd)
{
    // Setup
    const std::string keywordRecord = R"(KEYWORD = 'AB'''                 / COMMENT HERE                                 )";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, R"(AB')");
}

TEST(ParseKeywordValueAsString, FixedFormat_MaxLength)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 'ABCD                                                               E'";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD                                                               E");
}

TEST(ParseKeywordValueAsString, FixedFormat_NullString)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = ''                                                                    ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "");
}

TEST(ParseKeywordValueAsString, FixedFormat_EmptyString)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = '    '                                                                ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, " ");
}

TEST(ParseKeywordValueAsString, FixedFormat_NotAString)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 123                                                                   ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsString, FreeFormat_SameAsFixed)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 'ABCD'                                                                ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD");
}

TEST(ParseKeywordValueAsString, FreeFormat_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =           'ABCD'                                                      ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD");
}

TEST(ParseKeywordValueAsString, FreeFormat_WithComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =           'ABCD'                  / COMMENT HERE                      ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD");
}

TEST(ParseKeywordValueAsString, FreeFormat_EscapedChar)
{
    // Setup
    const std::string keywordRecord = R"(KEYWORD =           'AB''CD'               / COMMENT HERE                       )";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, R"(AB'CD)");
}

TEST(ParseKeywordValueAsString, FreeFormat_CloseAtEnd)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =           'ABCD                                                     E'";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD                                                     E");
}

TEST(ParseKeywordValueAsString, FreeFormat_FullSize)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 'ABCD                                                               E'";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "ABCD                                                               E");
}

TEST(ParseKeywordValueAsString, FreeFormat_NullString)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = ''                                                                    ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "");
}

TEST(ParseKeywordValueAsString, FreeFormat_EmptyString)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = '    '                                                                ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, " ");
}

TEST(ParseKeywordValueAsString, FreeFormat_NotAString)
{
    // Setup
    const std::string keywordRecord = "KEYWORD = 123                                                                   ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsString(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_FALSE(result);
}
