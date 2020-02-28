#pragma once

#include <glm/vec3.hpp>

namespace nova {
    struct Transform {
        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale{1};
    };
} // namespace nova::ec
