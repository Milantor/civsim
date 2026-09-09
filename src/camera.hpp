#pragma once

#include "raylib-cpp.hpp"

namespace civsim
{
    // camera: movement and zoom helpers.
    namespace camera
    {
        /**
         * Move the camera target with WASD and clamp the camera inside the level map.
         *
         * @param camera active camera object
         * @param worldWidth map width in pixels
         * @param worldHeight map height in pixels
         */
        void UpdateCameraMovement(raylib::Camera2D &camera,
                                  int worldWidth,
                                  int worldHeight);

        /**
         * Move through the fixed zoom steps with wheel input or numpad +/- keys.
         *
         * @param camera active camera object
         */
        void UpdateCameraZoom(raylib::Camera2D &camera);
    }
}