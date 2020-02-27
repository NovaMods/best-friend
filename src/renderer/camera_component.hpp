#pragma once
#include "nova_renderer/camera.hpp"

#include "../ec/component.hpp"
#include "../ec/transform.hpp"

namespace nova::bf {
    class CameraComponent final : public ec::Component {
    public:
        explicit CameraComponent(ec::Entity* owner, renderer::CameraAccessor camera);

        ~CameraComponent() override = default;

        void begin_play() override;

        void tick(double delta_time) override;

    private:
        renderer::CameraAccessor camera;
        ec::Transform* transform;
    };
} // namespace bf
