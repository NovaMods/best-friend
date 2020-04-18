#include "input.hpp"

#include <glm/ext/matrix_transform.inl>

#include "../ec/transform.hpp"
#include "GLFW/glfw3.h"
#include "rx/core/log.h"

namespace nova::bf {
    RX_LOG("InputRouter", logger);

    constexpr double ZOOM_SPEED = 1;

    InputRouter::InputRouter(entt::registry& registry, renderer::NovaWindow& window) : registry{registry} {
        window.register_key_callback(
            [&](const uint32_t key, const bool is_press, const bool /* is_control_down */, const bool /* is_shift_down */) {
                if(auto* val = keys.find(key); val != nullptr) {
                    *val = is_press;
                } else {
                    keys.insert(key, is_press);
                }
            });
    }

    void InputRouter::update_rotating_entities(const double delta_time) const {
        const auto& view = registry.view<RotateAroundPointCameraController, Transform>();

        const auto delta_zoom = [&] {
            if(auto* should_zoom_in = keys.find(GLFW_KEY_R); should_zoom_in != nullptr) {
                if(*should_zoom_in) {
                    return -ZOOM_SPEED * delta_time;
                }
            }
            return 0.0;
        }();

        for(auto entity : view) {
            auto& rotate_data = registry.get<RotateAroundPointCameraController>(entity);
            auto& transform = registry.get<Transform>(entity);

            // Adjust zoom
            rotate_data.distance -= delta_zoom;

            transform.position = {0, 0, -rotate_data.distance};
        }
    }
} // namespace nova::bf
