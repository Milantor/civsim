#include "level/io/save.hpp"

#include <cstdint>
#include <vector>

namespace civsim::level::io
{
    bool SaveLevelAsBinary(const Level &level,
                           std::vector<std::uint8_t> &bytes)
    {
        bytes.clear();
        // one byte per tile, so reserve exactly the tile count.
        bytes.reserve(level.tiles.size());

        // the tile field is already row-major, so just dump each tile as one byte.
        for (const tile::TileType tile : level.tiles)
        {
            bytes.push_back(static_cast<std::uint8_t>(tile));
        }

        return !bytes.empty();
    }

    bool SaveLevelAsPng(const Level &level,
                        const raylib::Texture2D &atlas,
                        raylib::Image &pngImage)
    {
        // pull the atlas texture into a CPU image once, so the blit loop is cheap.
        raylib::Image atlasImage = LoadImageFromTexture(atlas);

        // start from a blank canvas sized to the whole map in pixels.
        pngImage = GenImageColor(level.width * settings::TileSize,
                                 level.height * settings::TileSize,
                                 BLANK);

        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                const tile::TileType tile = level.At(x, y);
                const std::size_t index = static_cast<std::size_t>(tile);
                // skip tiles with no matching tile info slot.
                if (index >= tile::TileInfos.size())
                    continue;

                // source is the whole tile sprite inside the atlas, dest is its square on the map.
                const tile::SpriteInfo &sprite = tile::TileInfos[index].sprite;
                const Rectangle srcRec{sprite.u0 * atlasImage.width,
                                       sprite.v0 * atlasImage.height,
                                       (sprite.u1 - sprite.u0) * atlasImage.width,
                                       (sprite.v1 - sprite.v0) * atlasImage.height};
                const Rectangle dstRec{static_cast<float>(x * settings::TileSize),
                                       static_cast<float>(y * settings::TileSize),
                                       static_cast<float>(settings::TileSize),
                                       static_cast<float>(settings::TileSize)};

                ImageDraw(&pngImage,
                          atlasImage,
                          srcRec,
                          dstRec,
                          WHITE);
            }
        }

        return true;
    }
}
