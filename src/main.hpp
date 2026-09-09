#include "raylib-cpp.hpp"
#include "entt/entt.hpp"
#include "camera.hpp"
#include "level.hpp"
#include "settings.hpp"

/**
 * Draw the current tile map by blitting the configured tile textures.
 *
 * @param camera active camera object
 * @param level map object to draw
 * @param textures tile texture registry, indexed by TileType order
 */
void UpdateDrawFrame(raylib::Camera2D &camera,
                     const civsim::Level &level,
                     const std::array<raylib::Texture2D, 5> &textures);