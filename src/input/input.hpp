#pragma once
#include <entt/entity/registry.hpp>
#include <glm/vec3.hpp>

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

        /*!
         * \brief Current yaw around the focus point, in degrees
         */
        float cur_yaw{};

        /*!
         * \brief Current pitch around the focus point, in degrees
         */
        float cur_pitch{};
    };

    void update_rotating_cameras(entt::registry& registry, renderer::NovaWindow& window);
} // namespace nova::bf
