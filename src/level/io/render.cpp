#include "level/io/render.hpp"

namespace civsim::level::io
{
    void DrawLevel(const Level &level,
                   const raylib::Texture2D &atlas,
                   raylib::Vector2 origin)
    {
        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                const auto index = static_cast<std::size_t>(level.At(x, y));
                // skip tiles with no matching tile info slot.
                if (index >= tile::TileInfos.size())
                    continue;

                // blit the tile sprite from the atlas at its square on the map.
                const tile::SpriteInfo &sprite = tile::TileInfos[index].sprite;
                const Rectangle source{
                    sprite.u0 * atlas.width,
                    sprite.v0 * atlas.height,
                    (sprite.u1 - sprite.u0) * atlas.width,
                    (sprite.v1 - sprite.v0) * atlas.height};
                const Vector2 position{
                    origin.x + x * settings::TileSize,
                    origin.y + y * settings::TileSize};
                DrawTextureRec(atlas,
                               source,
                               position,
                               WHITE);
            }
        }
    }
}
