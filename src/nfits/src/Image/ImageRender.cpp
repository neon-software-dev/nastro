/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Image/ImageRender.h>

#include <cassert>

namespace NFITS
{

ImageRender::ImageRender(Format _format, std::size_t _width, std::size_t _height)
    : format(_format)
    , width(_width)
    , height(_height)
{
    imageBytes.resize(width * height * BytesPerPixel(), 0);
}

uint8_t ImageRender::BytesPerPixel() const noexcept
{
    switch (format)
    {
        case Format::RGB888: return 3;
    }

    assert(false);
    return 0;
}

unsigned char* ImageRender::GetScanLineBytesStart(const std::size_t& y)
{
    assert(y < height);
    return imageBytes.data() + (y * width * BytesPerPixel());
}

const unsigned char* ImageRender::GetScanLineBytesStart(const std::size_t& y) const
{
    assert(y < height);
    return imageBytes.data() + (y * width * BytesPerPixel());
}

unsigned char* ImageRender::GetPixelBytesStart(const size_t& x, const size_t& y)
{
    assert(x < width);
    assert(y < height);
    return imageBytes.data() + (y * width * BytesPerPixel()) + (x * BytesPerPixel());
}

const unsigned char* ImageRender::GetPixelBytesStart(const size_t& x, const size_t& y) const
{
    assert(x < width);
    assert(y < height);
    return imageBytes.data() + (y * width * BytesPerPixel()) + (x * BytesPerPixel());
}

}
