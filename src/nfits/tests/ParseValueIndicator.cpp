/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseValueIndicator, HappyPath)
{
    // Setup
    const std::array<char, 2> valueIndicator = {'=', ' '};

    // Act
    const auto result = ParseValueIndicator(valueIndicator);

    // Assert
    EXPECT_TRUE(result); // Value indicator detected
}

TEST(ParseValueIndicator, BadChars)
{
    // Setup
    const std::array<char, 2> valueIndicator = {0, 0};

    // Act
    const auto result = ParseValueIndicator(valueIndicator);

    // Assert
    EXPECT_FALSE(result); // Value indicator not detected
}

TEST(ParseValueIndicator, Spaces)
{
    // Setup
    const std::array<char, 2> valueIndicator = {' ', ' '};

    // Act
    const auto result = ParseValueIndicator(valueIndicator);

    // Assert
    EXPECT_FALSE(result); // Value indicator not detected
}

TEST(ParseValueIndicator, RandomValidChars)
{
    // Setup
    const std::array<char, 2> valueIndicator = {'A', 'B'};

    // Act
    const auto result = ParseValueIndicator(valueIndicator);

    // Assert
    EXPECT_FALSE(result); // Value indicator not detected
}
