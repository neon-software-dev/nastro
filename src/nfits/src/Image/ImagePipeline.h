/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_SRC_IMAGE_IMAGEPIPELINE_H
#define NFITS_SRC_IMAGE_IMAGEPIPELINE_H

#include <NFITS/Error.h>
#include <NFITS/Data/ImageData.h>

#include <vector>
#include <expected>
#include <memory>

namespace NFITS
{
    /**
     * Applies the physical value transform (zero + (scale * value)) to a collection of image values.
     */
    void ApplyPhysicalValueTransform(std::vector<double>& values, double zero, double scale);

    /**
     * Transforms raw from file, uncompressed, image values into physical values.
     *
     * Interprets the bytes as a sequence of values of data type determined by bitpix,
     * fixes the endianness of those values as needed, and applies the physical value
     * transform, returning the result as a collection of doubles.
     *
     * @param data The input image values
     * @param bitpix The bitpix of the raw data
     * @param bZero bZero to apply for the physical value transform
     * @param bScale bScale to apply for the physical value transform
     *
     * @return The resulting physical values, or Error on error
     */
    [[nodiscard]] std::expected<std::vector<double>, Error> RawImageDataToPhysicalValues(
        std::span<const std::byte> data,
        int64_t bitpix,
        double bZero,
        double bScale
    );

    /**
     * Transforms physical values into an ImageData.
     *
     * Note that it requires moving the physical values in, so the ImageData can be directly
     * constructed from the data rather than copying it.
     *
     * @param physicalValues The physical values to be transformed
     * @param sliceSpan Defines the organization of image slices within the physical values
     *
     * @return An ImageData, or Error upon error
     */
    [[nodiscard]] std::expected<std::unique_ptr<ImageData>, Error>
        PhysicalValuesToImageData(std::vector<double>&& physicalValues, const ImageSliceSpan& sliceSpan);
}

#endif //NFITS_SRC_IMAGE_IMAGEPIPELINE_H
