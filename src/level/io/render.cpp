#include "level/io/render.hpp"

#include <array>

namespace civsim::level::io
{
    void DrawLevel(const Level &level,
                   const std::array<raylib::Texture2D, 5> &textures,
                   raylib::Vector2 origin)
    {
        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                const auto index = static_cast<std::size_t>(level.At(x, y));
                // skip tiles with no matching texture slot.
                if (index >= textures.size())
                    continue;

                // blit the tile texture at its square on the map.
                const raylib::Texture2D &texture = textures[index];
                DrawTexture(texture,
                            static_cast<int>(origin.x + x * settings::TileSize),
                            static_cast<int>(origin.y + y * settings::TileSize),
                            WHITE);
            }
        }
    }
}
