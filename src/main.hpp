#pragma once

#include "raylib-cpp.hpp"
#include "entt/entt.hpp"
#include "camera.hpp"
#include "level/cursor.hpp"
#include "level/debug/tools.hpp"
#include "level/gen/factory.hpp"
#include "level/io/render.hpp"
#include "level/io/save.hpp"
#include "level/level.hpp"
#include "settings.hpp"

/**
 * Draw the current tile map by blitting tiles from the atlas.
 *
 * @param camera active camera object
 * @param level map object to draw
 * @param atlas tile texture atlas holding every tile sprite
 */
void UpdateDrawFrame(raylib::Camera2D &camera,
                     const civsim::level::Level &level,
                     const raylib::Texture2D &atlas);