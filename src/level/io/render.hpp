#pragma once

#include "level/level.hpp"
#include "raylib-cpp.hpp"
#include "settings.hpp"

#include <array>

// io: level serialization and rendering.
namespace civsim::level::io
{
    /**
     * Draw the map by walking the Level array and blitting textures per tile.
     *
     * @param level current map object
     * @param textures tile texture registry, indexed by TileType order
     * @param origin map origin used for sprite placement
     */
    void DrawLevel(const Level &level,
                   const std::array<raylib::Texture2D, 5> &textures,
                   raylib::Vector2 origin);
}
