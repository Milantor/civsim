#include "level/cursor.hpp"

#include <cmath>

namespace civsim::level::cursor
{
    std::optional<CursorTile> GetTileUnderCursor(const Level &level,
                                                 const raylib::Camera2D &camera,
                                                 raylib::Vector2 origin,
                                                 int tileSize)
    {
        // screen to world
        const float worldX = (GetMouseX() - camera.offset.x) / camera.zoom + camera.target.x;
        const float worldY = (GetMouseY() - camera.offset.y) / camera.zoom + camera.target.y;
        const int tileX = static_cast<int>(std::floor((worldX - origin.x) / tileSize));
        const int tileY = static_cast<int>(std::floor((worldY - origin.y) / tileSize));

        if (tileX < 0 || tileX >= level.width || tileY < 0 || tileY >= level.height)
            return std::nullopt;

        return CursorTile{tileX, tileY, level.At(tileX, tileY)};
    }
}
