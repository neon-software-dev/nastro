/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/HDU.h>
#include <NFITS/KeywordCommon.h>
#include <NFITS/Data/BinTableImageData.h>

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

bool HDU::ContainsAnyData() const
{
    return GetDataByteSize() > 0;
}

bool HDU::ContainsAnyTypeOfImageData() const
{
    return ContainsNormalImage() || ContainsBinTableImage();
}

bool HDU::ContainsNormalImage() const
{
    return ContainsAnyData() && type == HDU::Type::Image;
}

bool HDU::ContainsBinTableImage() const
{
    if (!ContainsAnyData())
    {
        return false;
    }

    if (type != HDU::Type::BinTable)
    {
        return false;
    }

    const auto record = header.GetFirstKeywordRecord(KEYWORD_NAME_ZIMAGE);
    if (!record)
    {
        return false;
    }

    const auto val = record->GetKeywordValue_AsLogical();
    if (!val)
    {
        return false;
    }

    return *val;
}

}
