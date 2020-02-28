#pragma once

#include <entt/entity/registry.hpp>

#include "nova_renderer/nova_renderer.hpp"
#include "nova_renderer/renderables.hpp"

namespace nova::bf {
    struct RenderableComponent {
        bool visible;
        renderer::RenderableId renderable;
    };

    /*!
     * \brief Updates the positions of all the renderables, ensuring that Nova will render everything with the correct transforms
     */
    void update_renderable_transforms(entt::registry& registry, renderer::NovaRenderer& renderer);
} // namespace nova::bf
