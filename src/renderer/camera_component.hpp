#pragma once

#include <entt/entt.hpp>
#include <nova_renderer/camera.hpp>

namespace nova::bf {
    struct CameraComponent {
        renderer::CameraAccessor camera;
    };

    /*!
     * \brief Updates the locations of all the cameras in the registry
     *
     * \param registry The registry that holds all the cameras
     */
    void update_camera_positions(entt::registry& registry);
} // namespace nova::bf
