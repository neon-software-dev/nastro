/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "Rice.h"

#include <ranges>
#include <algorithm>

namespace NFITS
{

//
// Code adapted from: https://github.com/astropy/astropy/blob/main/cextern/cfitsio/lib/ricecomp.c
//

///////////
// CFITSIO LICENSE
///////////

/*
Copyright (Unpublished--all rights reserved under the copyright laws of
the United States), U.S. Government as represented by the Administrator
of the National Aeronautics and Space Administration.  No copyright is
claimed in the United States under Title 17, U.S. Code.

Permission to freely use, copy, modify, and distribute this software
and its documentation without fee is hereby granted, provided that this
copyright notice and disclaimer of warranty appears in all copies.

DISCLAIMER:

THE SOFTWARE IS PROVIDED 'AS IS' WITHOUT ANY WARRANTY OF ANY KIND,
EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED TO,
ANY WARRANTY THAT THE SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE, AND FREEDOM FROM INFRINGEMENT, AND ANY WARRANTY THAT THE
DOCUMENTATION WILL CONFORM TO THE SOFTWARE, OR ANY WARRANTY THAT THE
SOFTWARE WILL BE ERROR FREE.  IN NO EVENT SHALL NASA BE LIABLE FOR ANY
DAMAGES, INCLUDING, BUT NOT LIMITED TO, DIRECT, INDIRECT, SPECIAL OR
CONSEQUENTIAL DAMAGES, ARISING OUT OF, RESULTING FROM, OR IN ANY WAY
CONNECTED WITH THIS SOFTWARE, WHETHER OR NOT BASED UPON WARRANTY,
CONTRACT, TORT , OR OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY
PERSONS OR PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED
FROM, OR AROSE OUT OF THE RESULTS OF, OR USE OF, THE SOFTWARE OR
SERVICES PROVIDED HEREUNDER.
 */

///////////
//ASTROPY LICENSE
///////////

/*
Copyright (c) 2011-2024, Astropy Developers

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    Neither the name of the Astropy Team nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///////////
// Start of adapted code
///////////

static const int nonzero_count[256] = {
    0,
    1,
    2, 2,
    3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

RiceCodec::RiceCodec(unsigned int blockSize)
    : m_blockSize(blockSize)
{

}

std::expected<std::vector<double>, Error> RiceCodec::Decompress(int64_t bytepix,
                                                                std::span<const std::byte> compressed,
                                                                std::size_t outputSize) const
{
    std::vector<double> output;
    output.reserve(outputSize);

    switch (bytepix)
    {
        case 4:
        {
            const auto decompressed = Decompress_32(compressed, outputSize);
            if (!decompressed)
            {
                return std::unexpected(decompressed.error());
            }

            std::ranges::transform(*decompressed, std::back_inserter(output), [](const auto& val){
                return static_cast<double>(val);
            });
        }
        break;

        default:
        {
            return std::unexpected(Error::Msg("Unsupported bytepix value: {}", bytepix));
        }
    }

    return output;
}

std::expected<std::vector<uint32_t>, Error> RiceCodec::Decompress_32(std::span<const std::byte> compressed,
                                                                     std::size_t outputSize) const
{
    const auto inputChars = std::span<const unsigned char>{
        reinterpret_cast<const unsigned char*>(compressed.data()),
        compressed.size()
    };

    std::vector<uint32_t> output;
    output.resize(outputSize);

    std::size_t i, imax;
    int k;
    int nbits, nzero, fs;
    const unsigned char *cend;
    unsigned char bytevalue;
    unsigned int b, diff, lastpix;
    int fsmax, fsbits, bbits;
    extern const int nonzero_count[];

    const int clen = static_cast<int>(compressed.size());
    fsbits = 5;
    fsmax = 25;

    bbits = 1 << fsbits;

    /* first 4 bytes of input buffer contain the value of the first */
    /* 4 byte integer value, without any encoding */
    if (clen < 4)
    {
        return std::unexpected(Error::Msg("decompression error: input buffer not properly allocated"));
    }

    lastpix = 0;
    bytevalue = inputChars[0];
    lastpix = lastpix | (unsigned int)((bytevalue<<24));
    bytevalue = inputChars[1];
    lastpix = lastpix | (unsigned int)((bytevalue<<16));
    bytevalue = inputChars[2];
    lastpix = lastpix | (unsigned int)((bytevalue<<8));
    bytevalue = inputChars[3];
    lastpix = lastpix | bytevalue;

    /*
    * Decode in blocks of nblock pixels
    */
    const unsigned char* c = &inputChars[0];
    c += 4;
    cend = c + clen - 4;

    b = *c++;		    /* bit buffer			*/
    nbits = 8;		    /* number of bits remaining in b	*/

    for (i = 0; i < outputSize; )
    {
        /* get the FS value from first fsbits */
        nbits -= fsbits;
        while (nbits < 0)
        {
            b = (b<<8) | (*c++);
            nbits += 8;
        }
        fs = (int)(b >> nbits) - 1;

        b &= (1 << nbits) - 1;

        /* loop over the next block */
        imax = i + m_blockSize;
        if (imax > outputSize)
        {
            imax = outputSize;
        }

        /* low-entropy case, all zero differences */
        if (fs<0)
        {
            for ( ; i<imax; i++)
            {
                output[i] = lastpix;
            }
        }
        /* high-entropy case, directly coded pixel values */
        else if (fs==fsmax)
        {
            for ( ; i<imax; i++)
            {
                k = bbits - nbits;
                diff = b<<k;
                for (k -= 8; k >= 0; k -= 8)
                {
                    b = *c++;
                    diff |= b << k;
                }

                if (nbits>0)
                {
                    b = *c++;
                    diff |= b >> (-k);
                    b &= (1 << nbits)-1;
                }
                else
                {
                    b = 0;
                }
                /*
                 * undo mapping and differencing
                 * Note that some of these operations will overflow the
                 * unsigned int arithmetic -- that's OK, it all works
                 * out to give the right answers in the output file.
                 */
                if ((diff & 1) == 0)
                {
                    diff = diff >> 1;
                }
                else
                {
                    diff = ~(diff >> 1);
                }

                output[i] = diff+lastpix;
                lastpix = output[i];
            }
        }
        /* normal case, Rice coding */
        else
        {
            for ( ; i<imax; i++)
            {
                /* count number of leading zeros */
                while (b == 0)
                {
                    nbits += 8;
                    b = *c++;
                }
                nzero = nbits - nonzero_count[b];
                nbits -= nzero+1;
                /* flip the leading one-bit */
                b ^= 1 << nbits;
                /* get the FS trailing bits */
                nbits -= fs;
                while (nbits < 0)
                {
                    b = (b << 8) | (*c++);
                    nbits += 8;
                }
                diff = (unsigned int)(nzero << fs) | (b >> nbits);
                b &= (1 << nbits)-1;

                /* undo mapping and differencing */
                if ((diff & 1) == 0)
                {
                    diff = diff >> 1;
                }
                else
                {
                    diff = ~(diff >> 1);
                }
                output[i] = diff+lastpix;
                lastpix = output[i];
            }
        }

        if (c > cend)
        {
            return std::unexpected(Error::Msg("decompression error: hit end of compressed byte stream"));
        }
    }

    if (c < cend)
    {
        return std::unexpected(Error::Msg("decompression warning: unused bytes at end of compressed buffer"));
    }

    return output;
}

}
