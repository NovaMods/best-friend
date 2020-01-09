#include "transform.hpp"

#include <algorithm>

namespace nova::ec {
    Transform::Transform(Entity* owner_in) : Component(owner_in) {
    }

    void Transform::set_parent(Transform* parent) {
        this->parent = parent;
        parent->add_child(this);
    }

    Transform* Transform::get_parent() const { return parent; }

    void Transform::add_child(Transform* child) {
        children.push_back(child);
        child->set_parent(this);
    }

    const std::vector<Transform*>& Transform::get_children() const { return children; }

    void Transform::remove_child(Transform* child)
    {
        const auto remove_loc = std::remove(children.begin(), children.end(), child);
        children.erase(remove_loc);
    }
} // namespace nova::ec
