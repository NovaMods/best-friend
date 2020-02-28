#include "train_rendering.hpp"

#include "../ec/transform.hpp"

namespace nova::bf {
    void update_renderable_transforms(entt::registry& registry, renderer::NovaRenderer& renderer) {
        auto view = registry.view<RenderableComponent, Transform>();

        for(auto entity : view) {
            const auto& transform = registry.get<Transform>(entity);
            const auto& renderable = registry.get<RenderableComponent>(entity);

            renderer::StaticMeshRenderableUpdateData update_data{};
            update_data.position = transform.position;
            update_data.rotation = transform.rotation;
            update_data.scale = transform.scale;
            update_data.visible = renderable.visible;

            renderer.update_renderable(renderable.renderable, update_data);
        }
    }
} // namespace nova::bf
