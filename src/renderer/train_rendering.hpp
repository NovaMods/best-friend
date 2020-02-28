#pragma once

#include <entt/entity/registry.hpp>

#include "nova_renderer/renderables.hpp"

#include "../ec/transform.hpp"

namespace nova::bf {
    struct RenderableComponent {
        bool enabled;
        renderer::RenderableId renderable;
    };

    inline void update_renderable_positions(entt::registry& registry) {
        auto view = registry.view<RenderableComponent, Transform>();

        for(auto entity : view) {
            const auto& transform = registry.get<Transform>(entity);
            //   const auto
        }
    }
} // namespace nova::bf
