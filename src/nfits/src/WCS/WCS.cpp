/*
 * SPDX-FileCopyrightText: 2025 Joe @ NEON Software
 *
 * SPDX-License-Identifier: MIT
 */
 
#include <NFITS/WCS/WCS.h>

#include "../Parsing.h"

#include "../Util/Compare.h"

#include <numbers>
#include <cmath>
#include <queue>
#include <unordered_set>
#include <cassert>

namespace NFITS
{

// Type 1 uses PCI_J and CDELTI keywords
std::vector<double> GetIntermediateWorldCoord_Type1(const std::vector<double>& pixelCoord, const WCSDescription& desc)
{
    assert(static_cast<std::size_t>(desc.j) == pixelCoord.size());

    std::vector<double> qis;

    // qi = SUM(j=1..N)[mij * (pj - rj)]
    for (int64_t i = 1; i <= desc.i; ++i)
    {
        double sum = 0.0;

        for (int64_t j = 1; j <= desc.j; ++j)
        {
            const double mij = desc.pci_j.at(i).at(j);
            const double rj = desc.crpixj.contains(j) ? desc.crpixj.at(j) : 0.0;
            double pj = pixelCoord.at(static_cast<std::size_t>(j - 1));

            // If CRPIX is 0, consider the reference points 0-based rather than normal
            // FITS 1-based; adjust the pixel coord we're using from 1-based to 0-based
            // to match
            if (AreEqual(rj, 0.0)) { pj--; }

            sum += mij * (pj - rj);
        }

        qis.push_back(sum);
    }

    // xi = si * qi
    std::vector<double> xis;

    for (int64_t i = 1; i <= desc.i; ++i)
    {
        const auto si = desc.cdelti.at(i);
        const auto qi = qis.at((std::size_t)(i - 1));

        xis.push_back(si * qi);
    }

    return xis;
}

// Type 2 uses CDI_J keyword
std::vector<double> GetIntermediateWorldCoord_Type2(const std::vector<double>& pixelCoord, const WCSDescription& desc)
{
    assert(static_cast<std::size_t>(desc.j) == pixelCoord.size());

    std::vector<double> xis;

    for (int64_t i = 1; i <= desc.i; ++i)
    {
        double sum = 0.0;

        for (int64_t j = 1; j <= desc.j; ++j)
        {
            // (cdi_j)(pj − rj)
            const auto cdij = desc.cdi_j.at(i).at(j);
            const double pj = pixelCoord.at(static_cast<std::size_t>(j - 1));
            const double rj = desc.crpixj.contains(j) ? desc.crpixj.at(j) : 0.0;

            sum += cdij * (pj - rj);
        }

        xis.push_back(sum);
    }

    return xis;
}

constexpr double DEG2RAD = std::numbers::pi / 180.0;
constexpr double RAD2DEG = 180.0 / std::numbers::pi;
constexpr double TAU = 2.0 * std::numbers::pi;

/**
 * Compute gnomonic projection of plane points to spherical points
 *
 * @param xi 1-based i for the RA/x keyword
 * @param x_d Projection plane component x value, in degrees
 * @param yi 1-based i for the DEC/y keyword
 * @param y_d Projection plane component y value, in degrees
 * @param desc The WCS description containing xi/yi parameters
 *
 * @return Projected RA/DEC, in degrees
 */
std::array<double, 2> GnomonicProjection(int64_t xi, double x_d, int64_t yi, double y_d, const WCSDescription& desc)
{
    // Projection plane point to be transformed, in radians
    const double x_r = x_d * DEG2RAD;
    const double y_r = y_d * DEG2RAD;

    // Reference values at reference point
    const double crvalXi = desc.crvali.contains(xi) ? desc.crvali.at(xi) : 0.0;
    const double ra0_r = crvalXi * DEG2RAD;

    const double crvalYi = desc.crvali.contains(yi) ? desc.crvali.at(yi) : 0.0;
    const double dec0_r = crvalYi * DEG2RAD;

    // Gnomonic projection
    const double den = std::cos(dec0_r) - (y_r * std::sin(dec0_r));

    double ra_r = ra0_r + std::atan(x_r / den);

    const double dec_r = std::atan(
        (std::sin(dec0_r) + (y_r * std::cos(dec0_r))) /
        (std::sqrt((den * den) + (x_r * x_r)))
    );

    // Normalize ra_r to [0, 2π) range
    while (ra_r < 0) { ra_r += TAU; }
    while (ra_r >= TAU) { ra_r -= TAU; }

    // Convert ra/dec to degrees
    const double ra_d = ra_r * RAD2DEG;
    const double dec_d = dec_r * RAD2DEG;

    return {ra_d, dec_d};
}

double LinearProjection(int64_t i, double xi, const WCSDescription& desc)
{
    const double crval = desc.crvali.contains(i) ? desc.crvali.at(i) : 0.0;

    return xi + crval;
}

std::expected<std::vector<double>, Error> PixelCoordToIntermediateWorldCoord(const std::vector<double>& pixelCoords, const WCSDescription& desc)
{
    if (!desc.pci_j.empty() && !desc.cdelti.empty())
    {
        return GetIntermediateWorldCoord_Type1(pixelCoords, desc);
    }
    else if (!desc.cdi_j.empty())
    {
        return GetIntermediateWorldCoord_Type2(pixelCoords, desc);
    }
    else
    {
        return std::unexpected(Error::Msg("Unsupported available WCS keywords"));
    }
}

std::expected<std::vector<WCSWorldCoord>, Error> PixelCoordToWorldCoords(const std::vector<double>& pixelCoords,
                                                                         const WCSDescription& desc)
{
    std::vector<WCSWorldCoord> worldCoords;

    //
    // Convert pixel coordinate to intermediate world coordinate, which is applicable for all WCS types
    //
    const auto iWorldCoord = PixelCoordToIntermediateWorldCoord(pixelCoords, desc);
    if (!iWorldCoord)
    {
        return std::unexpected(iWorldCoord.error());
    }

    //
    // Try to parse each i's CTYPE into a WCSCType.
    // std::nullopt if no CTYPE exists for an i.
    //
    std::vector<std::optional<WCSCType>> iTypes;

    for (int64_t i = 1; i <= desc.i; ++i)
    {
        if (desc.ctypei.contains(i))
        {
            const auto parsedCType = ParseWCSCType(desc.ctypei.at(i));
            if (!parsedCType)
            {
                return std::unexpected(parsedCType.error());
            }

            iTypes.emplace_back(*parsedCType);
        }
        else
        {
            iTypes.emplace_back(std::nullopt);
        }
    }

    std::unordered_set<int64_t> processedIs;

    //
    // Handle non-linear celestial pairs
    //
    const std::vector<std::pair<std::string, std::string>> celestialPairs = {
        {"RA", "DEC"}
    };

    const auto matchesNonLinearCoordinateType = [](const std::optional<WCSCType>& type, const std::string& coordinateType) {
        return
            type &&
            std::holds_alternative<WCSNonLinearCType>(*type) &&
            (std::get<WCSNonLinearCType>(*type).coordinateType == coordinateType);
    };

    for (const auto& it : celestialPairs)
    {
        //
        // Find is with matching non-linear coordinate types
        //
        const auto m1It = std::ranges::find_if(iTypes, [&](const std::optional<WCSCType>& type){
            return matchesNonLinearCoordinateType(type, it.first);
        });

        if (m1It == iTypes.cend())
        {
            continue;
        }

        const auto m1i = std::distance(iTypes.begin(), m1It) + 1;
        const auto m1 = std::get<WCSNonLinearCType>(**m1It);

        const auto m2It = std::ranges::find_if(iTypes, [&](const std::optional<WCSCType>& type){
            return matchesNonLinearCoordinateType(type, it.second);
        });

        if (m2It == iTypes.cend())
        {
            continue;
        }

        const auto m2i = std::distance(iTypes.begin(), m2It) + 1;
        const auto m2 = std::get<WCSNonLinearCType>(**m2It);

        //
        // Mark the is as processed, regardless of whether we actually support them below
        //
        processedIs.insert(m1i);
        processedIs.insert(m2i);

        //
        // Process the is depending on algorithm code
        //
        if (m1.algorithmCode == m2.algorithmCode)
        {
            if (m1.algorithmCode == "TAN")
            {
                const auto worldCoord = GnomonicProjection(
                    m1i, iWorldCoord->at((std::size_t)(m1i - 1)),
                    m2i, iWorldCoord->at((std::size_t)(m2i- 1)),
                    desc
                );

                worldCoords.push_back(WCSWorldCoord{.coordinateType = m1.coordinateType, .worldCoord = worldCoord.at(0)});
                worldCoords.push_back(WCSWorldCoord{.coordinateType = m2.coordinateType, .worldCoord = worldCoord.at(1)});
            }
        }
    }

    //
    // Handle linear, yet to be processed, i types
    //
    for (int64_t i = 1; i <= desc.i; ++i)
    {
        if (processedIs.contains(i))
        {
            continue;
        }

        const auto& type = iTypes.at(static_cast<std::size_t>(i - 1));

        const bool isLinearType = type && std::holds_alternative<WCSLinearCType>(*type);
        if (!isLinearType)
        {
            continue;
        }

        const auto linearType = std::get<WCSLinearCType>(*type);

        const auto worldCoord = LinearProjection(i, iWorldCoord->at((std::size_t)(i - 1)), desc);

        worldCoords.push_back(WCSWorldCoord{.coordinateType = linearType.coordinateType, .worldCoord = worldCoord});
    }

    return worldCoords;
}

std::expected<std::vector<WCSWorldCoord>, Error> PixelCoordToWorldCoords(const std::vector<double>& pixelCoords,
                                                                         const WCSParams& wcsParams)
{
    std::vector<WCSWorldCoord> worldCoords;

    for (const auto& desc : wcsParams.descriptions)
    {
        const auto descWorldCoords = PixelCoordToWorldCoords(pixelCoords, desc.second);
        if (!descWorldCoords)
        {
            return std::unexpected(descWorldCoords.error());
        }

        for (const auto& worldCoord : *descWorldCoords)
        {
            worldCoords.push_back(worldCoord);
        }
    }

    return worldCoords;
}

}
