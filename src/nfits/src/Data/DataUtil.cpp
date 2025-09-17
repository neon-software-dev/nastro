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

std::expected<std::unique_ptr<Data>, Error> LoadHDUDataBlocking(const FITSFile* pFile, const HDU* pHDU)
{
    switch (pHDU->type)
    {
        case HDU::Type::Image:
        {
            auto pImageData = ImageData::LoadFromFileBlocking(pFile, pHDU);
            if (!pImageData)
            {
                return std::unexpected(pImageData.error());
            }
            return pImageData;
        }

        default: return std::unexpected(Error::Msg(ErrorType::General, "LoadHDUDataBlocking: Unsupported HDU type"));
    }
}

}
