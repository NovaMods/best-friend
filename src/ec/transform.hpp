#pragma once
#include <vector>

#include "component.hpp"

namespace nova::ec {
	class Transform : public Component
	{
	public:
		Transform(Entity* owner);
		virtual ~Transform();

		void set_parent(Transform* parent);
		Transform* get_parent() const;

		void add_child(Transform* child);
		const std::vector<Transform*>& get_children() const;
		void remove_child(Transform* child);

	private:
		Transform* parent = nullptr;
		std::vector<Transform*> children;

		// TODO: Translation, rotation, scale
	};
}
