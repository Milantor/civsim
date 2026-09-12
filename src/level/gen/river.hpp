#pragma once

#include "level/level.hpp"
#include "level/seed.hpp"
#include "settings.hpp"

#include <cstdint>
#include <optional>

// river: river and sand bank generation.
namespace civsim::level::gen::river
{
    /**
     * One border point a river may start or end at.
     */
    struct RiverPoint
    {
        int x;
        int y;
    };

    /**
     * How GenerateRiver picks the start and end border points.
     */
    enum class RiverGeneratorMode : std::uint8_t
    {
        RandomEndpoints,
        FixedStartRandomEnd,
        FixedStartFixedEnd
    };

    /**
     * Paint the water body and sand bank onto the dirt field.
     *
     * @param level current map object
     * @param riverWidth river body width in tiles
     * @param bankWidth sand bank width in tiles
     * @return true when the river was painted
     */
    bool GenerateRiver(Level &level,
                       int riverWidth,
                       int bankWidth);

    /**
     * Paint the river from a border point list and endpoint mode.
     *
     * @param level current map object
     * @param riverWidth river body width in tiles
     * @param bankWidth sand bank width in tiles
     * @param mode endpoint selection mode
     * @param startPointIndex optional fixed start border point index
     * @param endPointIndex optional fixed end border point index
     * @return true when the river was painted
     */
    bool GenerateRiver(Level &level,
                       int riverWidth,
                       int bankWidth,
                       RiverGeneratorMode mode,
                       std::optional<int> startPointIndex = std::nullopt,
                       std::optional<int> endPointIndex = std::nullopt);
}
