/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef NFITS_INCLUDE_NFITS_WCS_WCS_H
#define NFITS_INCLUDE_NFITS_WCS_WCS_H

#include <NFITS/SharedLib.h>
#include <NFITS/Error.h>
#include <NFITS/WCS/WCSParams.h>

#include <utility>
#include <expected>
#include <string>

namespace NFITS
{
    struct WCSWorldCoord
    {
        std::string coordinateType;
        double worldCoord{0.0};
    };

    /**
     * Returns all world coords associated with a given pixel coordinate, across all WCS alternate descriptions
     *
     * @param pixelCoord Pixel coordinate in question, in FITS-standard space
     * @param wcsParams WCSParams containing WCS descriptions to be applied
     *
     * @return All WCS world coords associated with the pixel coord, or Error upon error. May be empty.
     */
    [[nodiscard]] NFITS_PUBLIC std::expected<std::vector<WCSWorldCoord>, Error> PixelCoordToWorldCoords(
        const std::vector<double>& pixelCoord,
        const WCSParams& wcsParams);
}

#endif //NFITS_INCLUDE_NFITS_WCS_WCS_H
