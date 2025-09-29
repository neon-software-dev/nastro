/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

using namespace NFITS;

TEST(ParseBinTableTFORMN, HappyPath)
{
    // Setup
    const std::string tformn = "1J";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->repeatCount, 1);
    EXPECT_EQ(result->type, BinFieldType::Integer32Bit);
    EXPECT_FALSE(result->arrayType);
}

TEST(ParseBinTableTFORMN, OnlyTypeSpecifier)
{
    // Setup
    const std::string tformn = "J";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->repeatCount, 1);
    EXPECT_EQ(result->type, BinFieldType::Integer32Bit);
    EXPECT_FALSE(result->arrayType);
}

TEST(ParseBinTableTFORMN, ZeroRepeat)
{
    // Setup
    const std::string tformn = "0J";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->repeatCount, 0);
    EXPECT_EQ(result->type, BinFieldType::Integer32Bit);
    EXPECT_FALSE(result->arrayType);
}

TEST(ParseBinTableTFORMN, NoTypeSpecifier)
{
    // Setup
    const std::string tformn = "1";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseBinTableTFORMN, EmptyString)
{
    // Setup
    const std::string tformn;

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseBinTableTFORMN, ArrayHappyPath)
{
    // Setup
    const std::string tformn = "1PB(558)";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->repeatCount, 1);
    EXPECT_EQ(result->type, BinFieldType::Array32Bit);
    ASSERT_TRUE(result->arrayType);
    EXPECT_EQ(*result->arrayType, BinFieldType::UnsignedByte);
    ASSERT_TRUE(result->arrayMaxCount);
    EXPECT_EQ(*result->arrayMaxCount, 558);
}

TEST(ParseBinTableTFORMN, ArrayMissingMaxCount)
{
    // Setup
    const std::string tformn = "1PB";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseBinTableTFORMN, ArrayNoRepeatCount)
{
    // Setup
    const std::string tformn = "PB(558)";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->repeatCount, 1);
    EXPECT_EQ(result->type, BinFieldType::Array32Bit);
    ASSERT_TRUE(result->arrayType);
    EXPECT_EQ(*result->arrayType, BinFieldType::UnsignedByte);
    ASSERT_TRUE(result->arrayMaxCount);
    EXPECT_EQ(*result->arrayMaxCount, 558);
}

TEST(ParseBinTableTFORMN, ArrayMissingFieldType)
{
    // Setup
    const std::string tformn = "1P(558)";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseBinTableTFORMN, ArrayWithArrayFieldType)
{
    // Setup
    const std::string tformn = "1PP(558)";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseBinTableTFORMN, ArrayOneDigitMaxCount)
{
    // Setup
    const std::string tformn = "1PB(9)";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->repeatCount, 1);
    EXPECT_EQ(result->type, BinFieldType::Array32Bit);
    ASSERT_TRUE(result->arrayType);
    EXPECT_EQ(*result->arrayType, BinFieldType::UnsignedByte);
    ASSERT_TRUE(result->arrayMaxCount);
    EXPECT_EQ(*result->arrayMaxCount, 9);
}

TEST(ParseBinTableTFORMN, ArrayNoDigitsMaxCount)
{
    // Setup
    const std::string tformn = "1PB()";

    // Act
    const auto result = ParseBinTable_TFORMN(tformn);

    // Assert
    ASSERT_FALSE(result);
}
