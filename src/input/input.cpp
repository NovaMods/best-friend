#include "input.hpp"

#include "../ec/transform.hpp"

namespace nova::bf {
    void update_rotating_cameras(entt::registry& registry, renderer::NovaWindow& window) {
        const auto& view = registry.view<RotateAroundPointCameraController, Transform>();

        for(auto entity : view) {
            auto& rotate_data = registry.get<RotateAroundPointCameraController>(entity);
            auto& transform = registry.get<Transform>(entity);
        }
    }
} // namespace nova::bf
