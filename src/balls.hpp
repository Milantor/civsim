#pragma once

#include "main.hpp"

namespace civsim
{
    // samples::balls: tiny ECS motion demo.
    namespace samples
    {
        namespace balls
        {
            // ENABLED:
            // old ECS marker from the ball demo era; keep it alive as a no-op tag.
            struct Enabled
            {
            };

            // ORBIT:
            // circle path shape from the older motion sample.
            struct Orbit
            {
                float radius;
                float angle;
                float angularSpeed;
            };

            // CIRCLE DATA:
            // color and size box for the old draw sample.
            struct CircleData
            {
                float radius;
                raylib::Color color;
            };

            // CreateCircleEntities() makes the old sample circle objects.
            void CreateCircleEntities(entt::registry &registry);

            // MoveEntities() steps the sample ECS motion.
            void MoveEntities(entt::registry &registry);

            // ToggleMovingEntities() flips the ball demo movement on/off.
            void ToggleMovingEntities(entt::registry &registry);
        }
    }
}