#pragma once

#include <functional>

#include <entt/entity/registry.hpp>

namespace nova::bf {
    template <typename... ComponentTypes>
    void update_system(entt::registry& registry, std::function<void(ComponentTypes&...)> func) {
        auto view = registry.view<ComponentTypes...>();

        for(auto entity : view) {
            func(registry.get<ComponentTypes>(entity)...);
        }
    }

} // namespace nova::bf
