#pragma once

#include <glm/vec3.hpp>

#include "rx/core/optional.h"

namespace nova {
    struct Transform {
        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale{1};
    };

    struct HierarchyMember {
        rx::optional<entt::entity> parent;
        rx::vector<entt::entity> children;
    };
} // namespace nova
