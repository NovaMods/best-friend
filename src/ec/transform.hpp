#pragma once

#include <glm/vec3.hpp>

namespace nova::ec {
    struct Transform {
        glm::vec3 position{};
        glm::vec3 rotation{};
    };
} // namespace nova::ec
