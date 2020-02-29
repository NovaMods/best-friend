#pragma once
#include <glm/vec3.hpp>
#include <entt/entity/registry.hpp>
#include "nova_renderer/window.hpp"

namespace nova::bf {
    /*!
     * \brief Says that an entity should rotate around a given point at a given distance
     */
    struct RotateAroundPointCameraController {
        /*!
         * \brief Point for the camera to rotate around
         */
        glm::vec3 focus_point{};

        /*!
         * \brief Distance from the focus point
         */
        float distance{};
    };

    void update_rotating_cameras(entt::registry& registry, renderer::NovaWindow& window);
}
