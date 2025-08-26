/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/Data/DataUtil.h>
#include <NFITS/Data/ImageData.h>
#include <NFITS/HDU.h>

namespace NFITS
{

std::expected<std::unique_ptr<Data>, bool> LoadHDUDataBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    switch (pHDU->type)
    {
        case HDU::Type::Image:
        {
            auto imageData = std::make_unique<ImageData>();
            if (!imageData->LoadFromFileBlocking(pFile, pHDU))
            {
                return std::unexpected("LoadHDUDataBlocking: Failed to load data");
            }
            return imageData;
        }

        default: return std::unexpected("LoadHDUDataBlocking: Unsupported HDU type");
    }
}

}
