/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/MemoryFITSByteSource.h>

#include <cstring>

namespace NFITS
{

std::expected<ByteSize, Error> MemoryFITSByteSource::GetByteSize() const
{
    return ByteSize{m_data.size()};
}

Result MemoryFITSByteSource::Resize(const ByteSize& byteSize)
{
    m_data.resize(byteSize.value);
    return Result::Success();
}

Result MemoryFITSByteSource::ReadBytes(std::span<std::byte> dst, const ByteOffset& byteOffset, const ByteSize& byteSize)
{
    if (dst.size() < byteSize.value)
    {
        return Result::Fail(ErrorType::General, "MemoryFITSByteSource::ReadBytes: dst size is too small for requested read");
    }

    if ((byteOffset.value + byteSize.value) > GetByteSize()->value)
    {
        return Result::Fail(ErrorType::General, "MemoryFITSByteSource::ReadBytes: Byte offset/size is out of bounds");
    }

    memcpy(dst.data(), m_data.data() + byteOffset.value, byteSize.value);

    return Result::Success();
}

Result MemoryFITSByteSource::WriteBytes(std::span<const std::byte> src, const ByteOffset& byteOffset, const ByteSize& byteSize, bool flush)
{
    if (src.size() < byteSize.value)
    {
        return Result::Fail(ErrorType::General, "MemoryFITSByteSource::WriteBytes: src size is too small for requested write");
    }

    if ((byteOffset.value + byteSize.value) > GetByteSize()->value)
    {
        return Result::Fail(ErrorType::General, "MemoryFITSByteSource::WriteBytes: Byte offset/size is out of bounds");
    }

    memcpy(m_data.data() + byteOffset.value, src.data(), byteSize.value);

    // Data written to memory is always "flushed", so ignore the flush param
    (void)flush;

    return Result::Success();
}

Result MemoryFITSByteSource::Flush()
{
    // no-op, in-memory bytes are always flushed

    return Result::Success();
}

}
