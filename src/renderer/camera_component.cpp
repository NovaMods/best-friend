#pragma once

#include "camera_component.hpp"
#include "../ec/transform.hpp"
#include "minitrace.h"

namespace nova::bf {
    void update_camera_positions(entt::registry& registry) {
        MTR_SCOPE("BestFriend", "update_camera_positions");
        auto view = registry.view<Transform, CameraComponent>();

        for(auto entity : view) {
            const auto& transform = registry.get<Transform>(entity);
            auto& camera = registry.get<CameraComponent>(entity);

            camera.camera->position = transform.position;
            camera.camera->rotation = transform.rotation;
        }
    }
} // namespace nova::bf
