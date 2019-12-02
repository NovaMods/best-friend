#include "entity.hpp"

#include "component.hpp"

namespace nova::ec {
    Entity::Entity() {
        id = next_id;
        next_id++;
    }

    uint64_t Entity::get_id() const { return id; }

    void Entity::tick(const double delta_time) {
        for(auto* component : components) {
            component->tick(delta_time);
        }
    }
} // namespace nova::ec
