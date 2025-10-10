/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseKeywordValueAsReal, FixedFormat_HappyPath)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =        2.8988638E+02                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("2.8988638E+02"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_NoFractional)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                  123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_NoFractional_WithPlus)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                 +123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_NoFractional_WithMinus)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                 -123                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("-123"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_BasicFractional)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =              123.456                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_BasicFractional_WithPlus)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =             +123.456                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_BasicFractional_WithMinus)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =             -123.456                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("-123.456"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_DecimalButNoFractional)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                 123.                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_E_PlusSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =          123.456E+10                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456E+10"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_E_MinusSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =          123.456E-10                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456E-10"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_D_PlusSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =          123.456D+10                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456D+10"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_D_MinusSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =          123.456D-10                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456D-10"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_E_NoSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =           123.456E10                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456E10"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_D_NoSign)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =           123.456D10                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("123.456D10"));
}

TEST(ParseKeywordValueAsReal, FixedFormat_NoDigits)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                                                                       ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsReal, FixedFormat_NoDigitsAfterE)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =             123.456E                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsReal, FixedFormat_NoDigitsAfterD)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =             123.456D                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsReal, FixedFormat_BadChars)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =             123.4e56                                                  ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseKeywordValueAsReal, FreeFormat_WithComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                          2.8988638E+02       / ABCD EFGH              ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("2.8988638E+02"));
}

TEST(ParseKeywordValueAsReal, FreeFormat_NoComment)
{
    // Setup
    const std::string keywordRecord = "KEYWORD =                          2.8988638E+02                                ";
    ASSERT_EQ(keywordRecord.length(), 80);

    // Act
    const auto result = ParseKeywordValue_AsReal(KeywordRecordCSpan(keywordRecord));

    // Assert
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(*result, std::stod("2.8988638E+02"));
}
