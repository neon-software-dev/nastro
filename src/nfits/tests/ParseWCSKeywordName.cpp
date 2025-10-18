/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <Parsing.h>

#include <array>

using namespace NFITS;

TEST(ParseWCSKeywordName_a, SimplePath)
{
    // Setup
    const std::string keywordName = "WCSAXES";

    // Act
    const auto result = ParseWCSKeywordName_a(keywordName, "WCSAXES");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "WCSAXES");
    EXPECT_FALSE(result->a);
    EXPECT_FALSE(result->j);
    EXPECT_FALSE(result->i);
}

TEST(ParseWCSKeywordName_a, With_a)
{
    // Setup
    const std::string keywordName = "WCSAXESA";

    // Act
    const auto result = ParseWCSKeywordName_a(keywordName, "WCSAXES");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "WCSAXES");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(*result->a, 'A');
    EXPECT_FALSE(result->j);
    EXPECT_FALSE(result->i);
}

TEST(ParseWCSKeywordName_a, MismatchedBaseName)
{
    // Setup
    const std::string keywordName = "WCSAXES";

    // Act
    const auto result = ParseWCSKeywordName_a(keywordName, "BAD");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_a, Bad_a)
{
    // Setup
    const std::string keywordName = "WCSAXESA";

    // Act
    const auto result = ParseWCSKeywordName_a(keywordName, "WCSAXES3");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_ia, SimplePath)
{
    // Setup
    const std::string keywordName = "CRVAL1";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CRVAL");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRVAL");
    EXPECT_FALSE(result->a);
    EXPECT_FALSE(result->j);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 1);
}

TEST(ParseWCSKeywordName_ia, With_a)
{
    // Setup
    const std::string keywordName = "CRVAL1A";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CRVAL");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRVAL");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(result->a, 'A');
    EXPECT_FALSE(result->j);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 1);
}

TEST(ParseWCSKeywordName_ia, Missing_i_and_a)
{
    // Setup
    const std::string keywordName = "CRVAL";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CRVAL");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_ia, Missing_i)
{
    // Setup
    const std::string keywordName = "CRVALA";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CRVAL");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_ia, DoubleDigit_i_no_a)
{
    // Setup
    const std::string keywordName = "CRVAL12";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CRVAL");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRVAL");
    ASSERT_FALSE(result->a);
    EXPECT_FALSE(result->j);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 12);
}

TEST(ParseWCSKeywordName_ia, DoubleDigit_i_with_a)
{
    // Setup
    const std::string keywordName = "CRVAL12A";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CRVAL");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRVAL");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(result->a, 'A');
    EXPECT_FALSE(result->j);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 12);
}

TEST(ParseWCSKeywordName_ja, SimplePath)
{
    // Setup
    const std::string keywordName = "CRPIX1";

    // Act
    const auto result = ParseWCSKeywordName_ja(keywordName, "CRPIX");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRPIX");
    EXPECT_FALSE(result->a);
    EXPECT_FALSE(result->i);
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 1);
}

TEST(ParseWCSKeywordName_ja, With_a)
{
    // Setup
    const std::string keywordName = "CRPIX1A";

    // Act
    const auto result = ParseWCSKeywordName_ja(keywordName, "CRPIX");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRPIX");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(result->a, 'A');
    EXPECT_FALSE(result->i);
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 1);
}

TEST(ParseWCSKeywordName_ja, Missing_j_and_a)
{
    // Setup
    const std::string keywordName = "CRPIX";

    // Act
    const auto result = ParseWCSKeywordName_ja(keywordName, "CRPIX");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_ja, Missing_j)
{
    // Setup
    const std::string keywordName = "CRPIXA";

    // Act
    const auto result = ParseWCSKeywordName_ja(keywordName, "CRPIX");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_ja, DoubleDigit_j_no_a)
{
    // Setup
    const std::string keywordName = "CRPIX12";

    // Act
    const auto result = ParseWCSKeywordName_ja(keywordName, "CRPIX");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRPIX");
    ASSERT_FALSE(result->a);
    EXPECT_FALSE(result->i);
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 12);
}

TEST(ParseWCSKeywordName_ja, DoubleDigit_j_with_a)
{
    // Setup
    const std::string keywordName = "CRPIX12A";

    // Act
    const auto result = ParseWCSKeywordName_ja(keywordName, "CRPIX");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CRPIX");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(result->a, 'A');
    EXPECT_FALSE(result->i);
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 12);
}

TEST(ParseWCSKeywordName_i, SimplePath)
{
    // Setup
    const std::string keywordName = "CROTA1";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CROTA");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CROTA");
    ASSERT_FALSE(result->a);
    EXPECT_FALSE(result->j);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 1);
}

TEST(ParseWCSKeywordName_i, DoubleDigit_i)
{
    // Setup
    const std::string keywordName = "CROTA12";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CROTA");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CROTA");
    ASSERT_FALSE(result->a);
    EXPECT_FALSE(result->j);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 12);
}

TEST(ParseWCSKeywordName_i, Missing_i)
{
    // Setup
    const std::string keywordName = "CROTA";

    // Act
    const auto result = ParseWCSKeywordName_ia(keywordName, "CROTA");

    // Assert
    ASSERT_FALSE(result);
}

TEST(ParseWCSKeywordName_i_ja, SimplePath)
{
    // Setup
    const std::string keywordName = "CD1_2";

    // Act
    const auto result = ParseWCSKeywordName_i_ja(keywordName, "CD");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CD");
    ASSERT_FALSE(result->a);
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 2);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 1);
}

TEST(ParseWCSKeywordName_i_ja, With_a)
{
    // Setup
    const std::string keywordName = "CD1_2A";

    // Act
    const auto result = ParseWCSKeywordName_i_ja(keywordName, "CD");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CD");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(result->a, 'A');
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 2);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 1);
}

TEST(ParseWCSKeywordName_i_ja, DoubleDigits_without_a)
{
    // Setup
    const std::string keywordName = "CD10_20";

    // Act
    const auto result = ParseWCSKeywordName_i_ja(keywordName, "CD");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CD");
    ASSERT_FALSE(result->a);
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 20);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 10);
}

TEST(ParseWCSKeywordName_i_ja, DoubleDigits_with_a)
{
    // Setup
    const std::string keywordName = "CD10_20A";

    // Act
    const auto result = ParseWCSKeywordName_i_ja(keywordName, "CD");

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(result->base, "CD");
    ASSERT_TRUE(result->a);
    EXPECT_EQ(result->a, 'A');
    ASSERT_TRUE(result->j);
    EXPECT_EQ(result->j, 20);
    ASSERT_TRUE(result->i);
    EXPECT_EQ(result->i, 10);
}
