#include "balls.hpp"

#include <cmath>

namespace civsim
{
    namespace samples
    {
        namespace balls
        {

            // --------- ECS SAMPLE CIRCLE ---------

            void CreateCircleEntities(entt::registry &registry)
            {
                const auto player = registry.create();
                registry.emplace<Enabled>(player);
                registry.emplace<Position>(player, raylib::Vector2{0.f, 0.f});
                registry.emplace<CircleData>(player, 24.f, YELLOW);

                for (int i = 0; i < 20; ++i)
                {
                    const auto entity = registry.create();
                    const float angle = static_cast<float>(i) * 2.f * PI / 20.f;
                    const float orbitRadius = 100.f + static_cast<float>(i % 4) * 55.f;
                    registry.emplace<Enabled>(entity);
                    registry.emplace<Position>(entity, raylib::Vector2{
                                                           std::cos(angle) * orbitRadius,
                                                           std::sin(angle) * orbitRadius});
                    registry.emplace<Orbit>(entity, orbitRadius, angle, 0.7f + static_cast<float>(i % 3) * 0.15f);
                    registry.emplace<CircleData>(entity, 10.f, RED);
                }
            }

            // --------- ECS SAMPLE MOTION TOGGLE ---------

            void ToggleMovingEntities(entt::registry &registry)
            {
                if (!raylib::Keyboard::IsKeyPressed(KEY_SPACE))
                    return;

                auto enabledView = registry.view<Orbit, Enabled>();
                for (auto [entity, orbit] : enabledView.each())
                    registry.remove<Enabled>(entity);

                auto disabledView = registry.view<Orbit>(entt::exclude<Enabled>);
                for (auto [entity, orbit] : disabledView.each())
                    registry.emplace<Enabled>(entity);
            }

            // --------- ECS SAMPLE MOTION UPDATE ---------

            void MoveEntities(entt::registry &registry)
            {
                auto view = registry.view<Position, Orbit, Enabled>();
                for (auto [entity, position, orbit] : view.each())
                {
                    orbit.angle += orbit.angularSpeed * GetFrameTime();
                    position.position = raylib::Vector2{
                        std::cos(orbit.angle) * orbit.radius,
                        std::sin(orbit.angle) * orbit.radius};
                }
            }

        }
    }
}