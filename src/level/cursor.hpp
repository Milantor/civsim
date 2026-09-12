#pragma once

#include "level/level.hpp"
#include "level/tile.hpp"
#include "raylib-cpp.hpp"

#include <optional>

// cursor: pointer-to-tile lookup for the level map.
namespace civsim::level::cursor
{
    // CURSOR TILE:

    /**
     * Tiny data box for the tile currently under the pointer.
     */
    struct CursorTile
    {
        int x;
        int y;
        tile::TileType type;
    };

    /**
     * Find the tile currently below the camera pointer.
     *
     * @param level current map object
     * @param camera current 2D camera view
     * @param origin map origin for world-space drawing
     * @param tileSize tile square size in pixels
     * @return optional cursor tile info when the pointer is inside the map
     */
    std::optional<CursorTile> GetTileUnderCursor(const Level &level,
                                                 const raylib::Camera2D &camera,
                                                 raylib::Vector2 origin,
                                                 int tileSize);
}
