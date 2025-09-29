/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/DiskFITSByteSource.h>

namespace NFITS
{

std::expected<std::unique_ptr<DiskFITSByteSource>, Error> DiskFITSByteSource::Open(const std::filesystem::path& filePath,
                                                                                   bool createIfNotExists)
{
    auto source = std::make_unique<DiskFITSByteSource>(Tag{}, filePath);

    const auto result = source->OpenStream(createIfNotExists);
    if (!result)
    {
        return std::unexpected(*result.error);
    }

    return source;
}

DiskFITSByteSource::DiskFITSByteSource(DiskFITSByteSource::Tag, std::filesystem::path filePath)
    : m_filePath(std::move(filePath))
{

}

std::expected<ByteSize, Error> DiskFITSByteSource::GetByteSize() const
{
    std::error_code ec{};

    const auto byteSize = std::filesystem::file_size(m_filePath, ec);
    if (ec)
    {
        return std::unexpected(Error::Msg("DiskFITSByteSource::GetByteSize: Call to file_size() failed"));
    }

    return ByteSize{byteSize};
}

Result DiskFITSByteSource::Resize(const ByteSize& byteSize)
{
    // Close the stream, flushing any pending data to it, before resizing
    CloseStream();

    // Resize
    std::error_code ec{};
    std::filesystem::resize_file(m_filePath, byteSize.value, ec);
    if (ec)
    {
        return Result::Fail("DiskFITSByteSource::Resize: Call to resize_file() failed");
    }

    // Re-open the stream
    return OpenStream(false);
}

Result DiskFITSByteSource::ReadBytes(std::span<std::byte> dst, const ByteOffset& byteOffset, const ByteSize& byteSize)
{
    if (!m_stream.is_open())
    {
        return Result::Fail("DiskFITSByteSource::ReadBytes: File stream isn't open");
    }

    if (dst.size() < byteSize.value)
    {
        return Result::Fail("DiskFITSByteSource::ReadBytes: dst size is too small for requested read");
    }

    // Seek to the specified byte offset
    m_stream.seekg(static_cast<std::streamoff>(byteOffset.value), std::ios::beg);
    if (!m_stream.good())
    {
        return Result::Fail("DiskFITSByteSource::ReadBytes: Failed to seek to byte offset");
    }

    // Read the specified number of bytes
    m_stream.read(reinterpret_cast<char*>(dst.data()), static_cast<std::streamsize>(byteSize.value));
    if (!m_stream.good())
    {
        return Result::Fail("DiskFITSByteSource::ReadBytes: Failed to read bytes from the stream");
    }

    return Result::Success();
}

Result DiskFITSByteSource::WriteBytes(std::span<const std::byte> src, const ByteOffset& byteOffset, const ByteSize& byteSize, bool flush)
{
    if (!m_stream.is_open())
    {
        return Result::Fail("DiskFITSByteSource::WriteBytes: File stream isn't open");
    }

    if (src.size() < byteSize.value)
    {
        return Result::Fail("DiskFITSByteSource::WriteBytes: src size is too small for requested write");
    }

    // Seek to the specified byte offset
    m_stream.seekp(static_cast<std::streamoff>(byteOffset.value));
    if (!m_stream.good())
    {
        return Result::Fail("DiskFITSByteSource::WriteBytes: Failed to seek to byte offset");
    }

    // Write the specified bytes
    m_stream.write(reinterpret_cast<const char*>(src.data()), static_cast<std::streamsize>(byteSize.value));
    if (!m_stream.good())
    {
        return Result::Fail("DiskFITSByteSource::WriteBytes: Failed to read bytes from the stream");
    }

    // Flush the stream, if requested
    if (flush)
    {
        m_stream.flush();
    }

    return Result::Success();
}

Result DiskFITSByteSource::Flush()
{
    if (!m_stream.is_open())
    {
        return Result::Fail("DiskFITSByteSource::Flush: File stream isn't open");
    }

    m_stream.flush();

    return Result::Success();
}

DiskFITSByteSource::~DiskFITSByteSource()
{
    CloseStream();
}

Result DiskFITSByteSource::OpenStream(bool createIfNotExists)
{
    // If the stream is already open, nothing to do
    if (m_stream.is_open())
    {
        return Result::Success();
    }

    // If the file doesn't exist, try to create it
    if (!std::filesystem::exists(m_filePath))
    {
        // Unless we're not supposed to
        if (!createIfNotExists)
        {
            return Result::Fail("DiskFITSByteSource: File doesn't exist");
        }

        std::ofstream createStream(m_filePath);
        if (!createStream.is_open())
        {
            // Failed to create the file
            return Result::Fail("DiskFITSByteSource: Failed to open/create the file");
        }

        createStream.close();
    }

    // Open the file for input/output
    m_stream = std::fstream(m_filePath, std::fstream::in | std::fstream::out | std::fstream::binary);
    if (!m_stream.is_open())
    {
        // Failed to open the file
        return Result::Fail("DiskFITSByteSource: Failed to open the file");
    }

    return Result::Success();
}

void DiskFITSByteSource::CloseStream()
{
    if (m_stream.is_open())
    {
        m_stream.close();
    }
}

}
