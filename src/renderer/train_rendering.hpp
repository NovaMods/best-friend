#pragma once

#include <entt/entity/registry.hpp>
#include <nova_renderer/nova_renderer.hpp>
#include <nova_renderer/renderables.hpp>

#include <nova_renderer/material.hpp>

namespace nova::bf {
    struct RenderableComponent {
        bool visible;
        renderer::RenderableId renderable;
    };


    struct TrainMaterial {
        // Materials use texture IDs. These IDs are _actually_ indices of the textures in the chonker texture array
        renderer::TextureId color_texture;
    };

    /*!
     * \brief Updates the positions of all the renderables, ensuring that Nova will render everything with the correct transforms
     */
    void update_renderable_transforms(entt::registry& registry, renderer::NovaRenderer& renderer);
} // namespace nova::bf
