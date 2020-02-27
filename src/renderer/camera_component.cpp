#include "camera_component.hpp"

#include "../ec/entity.hpp"
#include "../ec/transform.hpp"

namespace nova::bf {
    CameraComponent::CameraComponent(ec::Entity* owner, renderer::CameraAccessor camera) : Component{owner}, camera{camera} {}

    void CameraComponent::begin_play() {
        transform = owner->get_component<ec::Transform>();
        RX_ASSERT(transform != nullptr, "Entities with a CameraComponent MUST have a Transform");
    }

    void CameraComponent::tick(double /* delta_time */) {
        camera->position = transform->position;
        camera->rotation = transform->rotation;
    }
} // namespace nova::bf
