#pragma once

#include "raylib-cpp.hpp"
#include "level/cluster.hpp"
#include "level/seed.hpp"
#include "level/tile.hpp"
#include "settings.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace civsim::level
{
    // bring the tile module types into the level scope for the level core.
    using tile::TileType;
    // bring the cluster module type into the level scope for the level core.
    using cluster::RockCluster;

    // LEVEL:

    /**
     * Whole generated map object in memory: map size, seed, and the tile grid.
     */
    struct Level
    {
        int width;
        int height;
        std::uint64_t seed;

        /**
         * Build a level grid filled with dirt tiles.
         *
         * @param width map width in tiles
         * @param height map height in tiles
         * @param seed optional explicit seed; if omitted, a fresh random seed is created
         */
        Level(int width = settings::MapWidth,
              int height = settings::MapHeight,
              std::optional<std::uint64_t> seed = std::nullopt);

        std::vector<TileType> tiles;
        std::vector<RockCluster> rockClusters;

        /**
         * Write a tile by row-major map index.
         *
         * @param x tile column
         * @param y tile row
         * @return reference to the tile at (x, y)
         */
        TileType &At(int x, int y);

        /**
         * Read a tile by row-major map index.
         *
         * @param x tile column
         * @param y tile row
         * @return const reference to the tile at (x, y)
         */
        const TileType &At(int x, int y) const;
    };

}
