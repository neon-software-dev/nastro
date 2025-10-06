/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_DATA_BINTABLEIMAGEDATA_H
#define NFITS_INCLUDE_NFITS_DATA_BINTABLEIMAGEDATA_H

#include "ImageData.h"

#include "../SharedLib.h"

namespace NFITS
{
    /**
     * An ImageData sourced from compressed bin table data
     */
    class NFITS_PUBLIC BinTableImageData : public ImageData
    {
        public:

            BinTableImageData() = default;
            explicit BinTableImageData(ImageData&& imageData);

            ~BinTableImageData() override = default;
    };

    /**
     * Loads a compressed image from a bintable HDU
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<std::unique_ptr<BinTableImageData>, Error>
        LoadBinTableImageDataFromFileBlocking(const FITSFile* pFile, const HDU* pHDU);
}

#endif //NFITS_INCLUDE_NFITS_DATA_BINTABLEIMAGEDATA_H
