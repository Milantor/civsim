#include "camera.hpp"

#include "settings.hpp"

#include <algorithm>
#include <cmath>

namespace civsim
{
    namespace camera
    {

        // --------- CAMERA MOTION ---------

        void UpdateCameraMovement(raylib::Camera2D &camera, int worldWidth, int worldHeight)
        {
            // player move input
            raylib::Vector2 movement{0.f, 0.f};

            if (IsKeyDown(KEY_W))
                movement.y -= 1.f;
            if (IsKeyDown(KEY_S))
                movement.y += 1.f;
            if (IsKeyDown(KEY_A))
                movement.x -= 1.f;
            if (IsKeyDown(KEY_D))
                movement.x += 1.f;

            // same speed on slant moves
            const float movementLength = std::sqrt(movement.x * movement.x + movement.y * movement.y);
            if (movementLength > 0.f)
            {
                // move more at low zoom
                const float distance = civsim::settings::CameraMoveSpeed * GetFrameTime() / (movementLength * camera.zoom);
                camera.target.x += movement.x * distance;
                camera.target.y += movement.y * distance;
            }

            // keep cam in map
            const float halfViewWidth = GetScreenWidth() / (2.f * camera.zoom);
            const float halfViewHeight = GetScreenHeight() / (2.f * camera.zoom);
            const float halfWorldWidth = worldWidth / 2.f;
            const float halfWorldHeight = worldHeight / 2.f;

            camera.target.x = std::clamp(camera.target.x,
                                         -halfWorldWidth + halfViewWidth,
                                         halfWorldWidth - halfViewWidth);
            camera.target.y = std::clamp(camera.target.y,
                                         -halfWorldHeight + halfViewHeight,
                                         halfWorldHeight - halfViewHeight);
        }

        // --------- CAMERA ZOOM ---------

        void UpdateCameraZoom(raylib::Camera2D &camera)
        {
            // fixed zoom steps
            const auto &zoomLevels = civsim::settings::CameraZoomLevels;
            const int zoomLevelCount = static_cast<int>(zoomLevels.size());
            int zoomDirection = 0;

            // wheel and pad keys
            const float wheelMovement = GetMouseWheelMove();
            if (wheelMovement > 0.f)
                zoomDirection = 1;
            else if (wheelMovement < 0.f)
                zoomDirection = -1;

            if (IsKeyPressed(KEY_KP_ADD))
                zoomDirection = 1;
            else if (IsKeyPressed(KEY_KP_SUBTRACT))
                zoomDirection = -1;

            if (zoomDirection != 0)
            {
                // find close zoom step
                int currentLevel = 0;
                float smallestDifference = std::abs(camera.zoom - zoomLevels[currentLevel]);

                for (int i = 0; i < zoomLevelCount; ++i)
                {
                    const float difference = std::abs(camera.zoom - zoomLevels[i]);
                    if (difference < smallestDifference)
                    {
                        currentLevel = i;
                        smallestDifference = difference;
                    }
                }

                currentLevel = std::clamp(
                    currentLevel + zoomDirection,
                    0,
                    zoomLevelCount - 1);
                camera.zoom = zoomLevels[currentLevel];
            }
        }
    }

}