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

// CreateRandomTileEntity() makes a random sample tile entity for old ECS tests.
void CreateRandomTileEntity(entt::registry &registry,
                            const raylib::Texture2D &tileTexture);

// POSITION:
// old world position tag stored by the sample ECS code.
struct Position
{
    raylib::Vector2 position;
};

// SPRITE DATA:
// pointer to the texture that should be used by the older draw pass.
struct SpriteData
{
    const raylib::Texture2D *texture;
};