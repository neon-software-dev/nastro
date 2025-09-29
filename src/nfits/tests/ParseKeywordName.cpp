/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseKeywordName, BadChars)
{
    // Setup
    const std::array<char, 8> keywordName = {0, ' ', ' ', ' ', ' ', ' ', ' ', ' '};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordName, BadChars2)
{
    // Setup
    const std::array<char, 8> keywordName = {'a', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordName, GoodChars)
{
    // Setup
    const std::array<char, 8> keywordName = {'5', 'G', '_', '-', ' ', ' ', ' ', ' '};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(*result);
    EXPECT_EQ(**result, "5G_-");
}

TEST(ParseKeywordName, LeadingSpace)
{
    // Setup
    const std::array<char, 8> keywordName = {' ', 'A', 'A', 'A', 'A', 'A', 'A', 'A'};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordName, TrailingSpaces)
{
    // Setup
    const std::array<char, 8> keywordName = {'A', 'A', 'A', 'A', ' ', ' ', ' ', ' '};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(*result);
    EXPECT_EQ(**result, "AAAA");
}

TEST(ParseKeywordName, EmbeddedSpaces)
{
    // Setup
    const std::array<char, 8> keywordName = {'A', ' ', 'A', 'A', 'A', 'A', 'A', 'A'};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_FALSE(result);
}

TEST(ParseKeywordName, FullLength)
{
    // Setup
    const std::array<char, 8> keywordName = {'A', 'A', 'A', 'A', 'A', 'A', 'A', 'Z'};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_TRUE(*result);
    EXPECT_EQ(**result, "AAAAAAAZ");
}

TEST(ParseKeywordName, AllSpaces)
{
    // Setup
    const std::array<char, 8> keywordName = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

    // Act
    const auto result = ParseKeywordName(keywordName);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_FALSE(*result);
}
