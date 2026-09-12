#include "level/io/save.hpp"

#include <array>
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
                        const std::array<raylib::Texture2D, 5> &textures,
                        Image &pngImage)
    {
        // pull every tile texture into a CPU image once, so the blit loop is cheap.
        std::array<Image, 5> images{};
        for (std::size_t i = 0; i < images.size(); ++i)
            images[i] = LoadImageFromTexture(textures[i]);

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
                // skip tiles with no matching texture slot.
                if (index >= textures.size())
                    continue;

                // source is the whole tile image, dest is its square on the map.
                Image src = images[index];
                const Rectangle srcRec{0.f, 0.f,
                                       static_cast<float>(src.width),
                                       static_cast<float>(src.height)};
                const Rectangle dstRec{static_cast<float>(x * settings::TileSize),
                                       static_cast<float>(y * settings::TileSize),
                                       static_cast<float>(settings::TileSize),
                                       static_cast<float>(settings::TileSize)};

                ImageDraw(&pngImage,
                          src,
                          srcRec,
                          dstRec,
                          WHITE);
            }
        }

        // free the temporary CPU images.
        for (Image &image : images)
            UnloadImage(image);

        return true;
    }
}
