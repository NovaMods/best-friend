#pragma once

#include <entt/entity/registry.hpp>

namespace nova {
    namespace renderer {
        class NovaRenderer;
    }
} // namespace nova

namespace nova::bf {
    struct LoadTrainEvent;

    class DataLoader {
    public:
        DataLoader(entt::registry& world, renderer::NovaRenderer& renderer_in);

        void load_train(const LoadTrainEvent& event) const;

    private:
        entt::registry& registry;
        renderer::NovaRenderer& renderer;
    };
} // namespace nova::bf
