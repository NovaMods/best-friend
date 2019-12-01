#include "entity.hpp"

#include "component.hpp"

namespace  nova::ec {
    void Entity::tick(const double delta_time)
	{
		for(auto* component : components)
		{
			component->tick(delta_time);
		}
	}
}
