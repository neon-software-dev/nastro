/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "DataUtil.h"
#include "ImageData.h"

#include <NFITS/IFITSByteSource.h>
#include <NFITS/Def.h>

namespace Nastro
{

std::expected<std::unique_ptr<Data>, bool> LoadHDUDataBlocking(const NFITS::FITSFile* pFile, const NFITS::HDU* pHDU)
{
    const auto dataByteStart = pHDU->GetDataBlockStartIndex() * NFITS::BLOCK_BYTE_SIZE.value;
    const auto dataByteSize = pHDU->GetDataByteSize();

    std::vector<std::byte> dataBytes{};
    dataBytes.resize(dataByteSize);

    pFile->GetByteSource()->ReadBytes(dataBytes, NFITS::ByteOffset(dataByteStart), NFITS::ByteSize(dataByteSize));

    // TODO: Determine data type

    auto data = ImageData::FromData(pHDU, std::move(dataBytes));
    if (!data)
    {
        return std::unexpected(data.error());
    }

    return std::move(*data);
}

}
