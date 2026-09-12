#pragma once

#include "level/level.hpp"
#include "level/tile.hpp"
#include "raylib-cpp.hpp"
#include "settings.hpp"

#include <cstdint>
#include <vector>

// io: level serialization and rendering.
namespace civsim::level::io
{
    /**
     * Make a binary byte stream from the Level vector.
     *
     * @param level current map object
     * @param bytes output byte vector for generatedlevel.dat
     * @return true when the byte vector is filled
     */
    bool SaveLevelAsBinary(const Level &level,
                           std::vector<std::uint8_t> &bytes);

    /**
     * Build a PNG image from the Level object using a single texture atlas.
     *
     * @param level current map object
     * @param atlas tile texture atlas holding every tile sprite
     * @param pngImage output image buffer for the PNG file
     * @return true when the Image buffer is filled
     */
    bool SaveLevelAsPng(const Level &level,
                        const raylib::Texture2D &atlas,
                        Image &pngImage);
}
