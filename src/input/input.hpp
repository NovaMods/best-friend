#pragma once

#include <entt/entity/registry.hpp>
#include <glm/vec3.hpp>
#include <nova_renderer/window.hpp>
#include <rx/core/map.h>

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
        float distance{10};

        /*!
         * \brief Current yaw around the focus point, in degrees
         */
        float cur_yaw{};

        /*!
         * \brief Current pitch around the focus point, in degrees
         */
        float cur_pitch{};
    };

    /*!
     * \brief Class that routes input from the window to relevant components
     */
    class InputRouter {
    public:
        InputRouter(entt::registry& registry, renderer::NovaWindow& window);

        void update_rotating_entities(const double delta_time) const;

    private:
        entt::registry& registry;
        renderer::NovaWindow& window;

        rx::map<uint32_t, bool> keys;
    };
} // namespace nova::bf
