/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_IMAGE_IMAGERENDER_H
#define NFITS_INCLUDE_NFITS_IMAGE_IMAGERENDER_H

#include "../SharedLib.h"

#include <vector>
#include <cstdint>

namespace NFITS
{
    /**
     * Stores the pixel components/values for a rendered image.
     *
     * Image bytes are stored tightly packed, bottom to top.
     */
    struct NFITS_PUBLIC ImageRender
    {
        enum class Format
        {
            RGB888 // RGB, 1 byte per component
        };

        ImageRender() = default;
        ImageRender(Format _format, std::size_t _width, std::size_t _height);

        [[nodiscard]] uint8_t BytesPerPixel() const noexcept;

        [[nodiscard]] unsigned char* GetScanLineBytesStart(const std::size_t& y);
        [[nodiscard]] const unsigned char* GetScanLineBytesStart(const std::size_t& y) const;

        [[nodiscard]] unsigned char* GetPixelBytesStart(const std::size_t& x, const std::size_t& y);
        [[nodiscard]] const unsigned char* GetPixelBytesStart(const std::size_t& x, const std::size_t& y) const;

        //

        Format format{Format::RGB888};
        std::size_t width{0};
        std::size_t height{0};
        std::vector<unsigned char> imageBytes;
    };
}

#endif //NFITS_INCLUDE_NFITS_IMAGE_IMAGERENDER_H
