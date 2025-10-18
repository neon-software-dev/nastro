/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <gtest/gtest.h>

#include <NFITS/Image/ImageSlice.h>

using namespace NFITS;


TEST(GetNumSlicesInSpan, HappyPath)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};

    // Act
    const auto result = GetNumSlicesInSpan(span);

    // Assert
    EXPECT_EQ(1000, result);
}

TEST(SliceKeyToLinearIndex, HappyPath)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto key = ImageSliceKey{.axesValues = {5, 5, 5}};

    // Act
    const auto result = SliceKeyToLinearIndex(span, key);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 555);
}

TEST(SliceKeyToLinearIndex, NoKeyValues)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto key = ImageSliceKey{};

    // Act
    const auto result = SliceKeyToLinearIndex(span, key);

    // Assert
    ASSERT_FALSE(result);
}

TEST(SliceKeyToLinearIndex, TwoDimenSpan)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100}};
    const auto key = ImageSliceKey{};

    // Act
    const auto result = SliceKeyToLinearIndex(span, key);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 0);
}

TEST(SliceKeyToLinearIndex, MissingKeyValues1)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto key = ImageSliceKey{.axesValues = {5}};

    // Act
    const auto result = SliceKeyToLinearIndex(span, key);

    // Assert
    ASSERT_FALSE(result);
}

TEST(SliceKeyToLinearIndex, MaxValue)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto key = ImageSliceKey{.axesValues = {9, 9, 9}};

    // Act
    const auto result = SliceKeyToLinearIndex(span, key);

    // Assert
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 999);
}

TEST(SliceLinearIndexToKey, HappyPath)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto index = 555U;

    // Act
    const auto result = SliceLinearIndexToKey(span, index);

    // Assert
    ASSERT_TRUE(result);

    const auto key = ImageSliceKey{.axesValues = {5, 5, 5}};
    EXPECT_EQ(*result, key);
}

TEST(SliceLinearIndexToKey, TwoDimensionalSpan)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100}};
    const auto index = 0U;

    // Act
    const auto result = SliceLinearIndexToKey(span, index);

    // Assert
    ASSERT_TRUE(result);

    const auto key = ImageSliceKey{};
    EXPECT_EQ(*result, key);
}

TEST(SliceLinearIndexToKey, ZeroIndex)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto index = 0U;

    // Act
    const auto result = SliceLinearIndexToKey(span, index);

    // Assert
    ASSERT_TRUE(result);

    const auto key = ImageSliceKey{.axesValues = {0, 0, 0}};
    EXPECT_EQ(*result, key);
}

TEST(SliceLinearIndexToKey, MaxValue)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto index = 999U;

    // Act
    const auto result = SliceLinearIndexToKey(span, index);

    // Assert
    ASSERT_TRUE(result);

    const auto key = ImageSliceKey{.axesValues = {9, 9, 9}};
    EXPECT_EQ(*result, key);
}

TEST(SliceLinearIndexToKey, OutOfBoundsIndex)
{
    // Setup
    const auto span = ImageSliceSpan{.axes = {100, 100, 10, 10, 10}};
    const auto index = 1000U;

    // Act
    const auto result = SliceLinearIndexToKey(span, index);

    // Assert
    ASSERT_FALSE(result);
}
