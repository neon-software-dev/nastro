/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/FITSBlockSource.h>
#include <NFITS/IFITSByteSource.h>
#include <NFITS/Def.h>

namespace NFITS
{

FITSBlockSource::FITSBlockSource(IFITSByteSource* pByteSource)
    : m_pByteSource(pByteSource)
{

}

FITSBlockSource::~FITSBlockSource() = default;

std::expected<uintmax_t, Error> FITSBlockSource::GetNumBlocks() const
{
    const auto byteSize = m_pByteSource->GetByteSize();
    if (!byteSize)
    {
        return std::unexpected(byteSize.error());
    }

    if (*byteSize % BLOCK_BYTE_SIZE != 0)
    {
        return std::unexpected(Error::Msg("FITSBlockSource::GetNumBlocks: Source byte size isn't a multiple of block size"));
    }

    return (*byteSize / BLOCK_BYTE_SIZE).value;
}

Result FITSBlockSource::ResizeBlocks(uintmax_t numBlocks)
{
    return m_pByteSource->Resize(BLOCK_BYTE_SIZE * numBlocks);
}

Result FITSBlockSource::ReadBlock(BlockSpan dst, const uintmax_t& blockIndex)
{
    return m_pByteSource->ReadBytes(dst, ByteOffset(BLOCK_BYTE_SIZE * blockIndex), BLOCK_BYTE_SIZE);
}

Result FITSBlockSource::WriteBlock(BlockCSpan src, const uintmax_t& blockIndex, bool flush)
{
    return m_pByteSource->WriteBytes(src, ByteOffset(BLOCK_BYTE_SIZE * blockIndex), BLOCK_BYTE_SIZE, flush);
}

}
