/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/HDU.h>

namespace NFITS
{

uintmax_t HDU::GetTotalBlockCount() const
{
    return GetHeaderBlockCount() + GetDataBlockCount();
}

uintmax_t HDU::GetHeaderBlockStartIndex() const
{
    return blockStartIndex;
}

uintmax_t HDU::GetHeaderBlockCount() const
{
    return header.headerBlocks.size();
}

uintmax_t HDU::GetDataBlockStartIndex() const
{
    return GetHeaderBlockStartIndex() + header.headerBlocks.size();
}

uintmax_t HDU::GetDataBlockCount() const
{
    return numDataBlocks;
}

uintmax_t HDU::GetDataByteSize() const
{
    return dataByteSize;
}

}
