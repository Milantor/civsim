#include "level/level.hpp"

#include <vector>

namespace civsim::level
{
    // --------- LEVEL MODEL ---------

    // level starts with a dirt tile grid.
    Level::Level(int width,
                 int height,
                 std::optional<std::uint64_t> seed)
        : width(width),
          height(height),
          seed(seed.value_or(seed::MakeRandomSeed())),
          tiles(std::vector<TileType>(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), TileType::Dirt))
    {
    }

    TileType &Level::At(int x, int y)
    {
        return tiles[static_cast<std::size_t>(y * width + x)];
    }

    const TileType &Level::At(int x, int y) const
    {
        return tiles[static_cast<std::size_t>(y * width + x)];
    }

}