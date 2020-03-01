#include "input.hpp"

#include "../ec/transform.hpp"
#include "rx/core/log.h"

namespace nova::bf {
    RX_LOG("InputRouter", logger);

    InputRouter::InputRouter(entt::registry& registry, renderer::NovaWindow& window) : registry{registry}, window{window} {
        window.register_key_callback([&](const uint32_t key, const bool is_press, const bool is_control_down, const bool is_shift_down) {
            logger(rx::log::level::k_info, "Pressed key %u", key);
        });
    }

    void InputRouter::update_rotating_entities() const {
        const auto& view = registry.view<RotateAroundPointCameraController, Transform>();

        for(auto entity : view) {
            auto& rotate_data = registry.get<RotateAroundPointCameraController>(entity);
            auto& transform = registry.get<Transform>(entity);
        }
    }
} // namespace nova::bf
