#pragma once
#include <vector>

#include <glm/vec3.hpp>

#include "component.hpp"

namespace nova::ec {
    class Transform final : public Component {
    public:
        glm::vec3 position{};
        glm::vec3 rotation{};

        explicit Transform(Entity* owner_in);
        virtual ~Transform() = default;

        void set_parent(Transform* parent);
        [[nodiscard]] Transform* get_parent() const;

        void add_child(Transform* child);
        [[nodiscard]] const std::vector<Transform*>& get_children() const;
        void remove_child(Transform* child);

    private:
        Transform* parent = nullptr;
        std::vector<Transform*> children;

        // TODO: Translation, rotation, scale
    };
} // namespace nova::ec
