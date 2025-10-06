/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_HDU_H
#define NFITS_INCLUDE_NFITS_HDU_H

#include "Header.h"
#include "SharedLib.h"

#include <vector>
#include <cstdint>

namespace NFITS
{
    /**
     * Holds metadata about a FITS HDU; mainly the headers contained in it, and metadata
     * needed to locate/load the HDU's data. Does not hold any of the HDU's data, only its
     * headers.
     */
    struct NFITS_PUBLIC HDU
    {
        enum class Type
        {
            Image,
            Table,
            BinTable
        };

        /**
         * @return The total number of blocks in the HDU - both header and data blocks
         */
        [[nodiscard]] uintmax_t GetTotalBlockCount() const;

        /**
         * @return The block index, within the FITS file, of the HDU's first header block
         */
        [[nodiscard]] uintmax_t GetHeaderBlockStartIndex() const;

        /**
         * @return The number of header blocks the HDU contains
         */
        [[nodiscard]] uintmax_t GetHeaderBlockCount() const;

        /**
         * @return The block index, within the FITS file, of the HDU's first data block
         */
        [[nodiscard]] uintmax_t GetDataBlockStartIndex() const;

        /**
        * @return The number of data blocks the HDU contains
        */
        [[nodiscard]] uintmax_t GetDataBlockCount() const;

        /**
        * @return The total byte size of all the HDU's data blocks
        */
        [[nodiscard]] uintmax_t GetDataByteSize() const;

        //////////////////

        /**
         * Whether this HDU is the FITS file's primary HDU
         */
        bool isPrimary{false};

        /**
         * The type of data the HDU contains
         */
        Type type{Type::Image};

        /**
         * The block index, within the FITS file, of the HDU's first block
         */
        uintmax_t blockStartIndex{0};

        /**
         * The header data associated with this HDU
         */
        Header header{};

        /**
         * The total number of data blocks the HDU contains
         */
        uintmax_t numDataBlocks{0};

        /**
        * The total byte size of the data associated with the HDU. Note that this
        * is not the byte size of the data blocks containing the data, it's the
        * size of the actual data.
        */
        uintmax_t dataByteSize{0};

        /**
         * @return Whether the HDU contains any data. There's valid scenarios where
         * an HDU can be configured as an "empty" image with no data, with subsequent
         * extensions HDUs containing the file's actual data.
         */
        [[nodiscard]] bool ContainsAnyData() const;

        /**
         * @return Whether the HDU contains any image data, whether via image or
         * compressed bintable image data.
         */
        [[nodiscard]] bool ContainsAnyTypeOfImageData() const;

        /**
         * @return Whether the HDU contains a normal image, exclusing compressed
         * bintable images.
         */
        [[nodiscard]] bool ContainsNormalImage() const;

        /**
         * @return Whether the HDU specifically contains compressed bintable image data.
         */
        [[nodiscard]] bool ContainsBinTableImage() const;
    };
}

#endif //NFITS_INCLUDE_NFITS_HDU_H
