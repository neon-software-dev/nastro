/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Data/DataUtil.h>
#include <NFITS/Data/ImageData.h>
#include <NFITS/Data/BinTableImageData.h>
#include <NFITS/HDU.h>

namespace NFITS
{

std::expected<std::unique_ptr<Data>, Error> LoadHDUDataBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    if (pHDU->ContainsNormalImage())
    {
        auto pData = LoadImageDataFromFileBlocking(pFile, pHDU);
        if (!pData)
        {
            return std::unexpected(pData.error());
        }
        return pData;
    }
    else if (pHDU->ContainsBinTableImage())
    {
        auto pData = LoadBinTableImageDataFromFileBlocking(pFile, pHDU);
        if (!pData)
        {
            return std::unexpected(pData.error());
        }
        return pData;
    }

    return std::unexpected(Error::Msg("LoadHDUDataBlocking: Unsupported HDU type"));
}

}
