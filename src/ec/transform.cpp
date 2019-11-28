#include "transform.hpp"

namespace nova::ec {
	void Transform::set_parent(Transform* parent)
	{
		this->parent = parent;
		parent->add_child(this);
	}

	Transform* Transform::get_parent() const
	{
		return parent;
	}

	void Transform::add_child(Transform* child)
	{
		children.push_back(child);
		child->set_parent(this);
	}

	const std::vector<Transform*>& Transform::get_children() const
	{
		return children;
	}
}
