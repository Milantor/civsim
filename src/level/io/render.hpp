#pragma once

#include "level/level.hpp"
#include "level/tile.hpp"
#include "raylib-cpp.hpp"
#include "settings.hpp"

// io: level serialization and rendering.
namespace civsim::level::io
{
    /**
     * Draw the map by walking the Level array and blitting tiles from the atlas.
     *
     * @param level current map object
     * @param atlas tile texture atlas holding every tile sprite
     * @param origin map origin used for sprite placement
     */
    void DrawLevel(const Level &level,
                   const raylib::Texture2D &atlas,
                   raylib::Vector2 origin);
}
