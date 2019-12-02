#include "world.hpp"

#include <algorithm>

#include "../ec/entity.hpp"

void World::add_entity(nova::ec::Entity* entity) {
    entity->world = this;

    entities.push_back(entity);
}

void World::remove_entity(nova::ec::Entity* entity) {
    const auto itr = std::remove_if(entities.begin(), entities.end(), [&](const auto* entity_to_test) {
        return entity_to_test->get_id() == entity->get_id();
    });

    if(itr) {
        itr->world = nullptr;
        entities.erase(itr);
    }
}

void World::tick(const double delta_time) {
    for(const auto* entity : entities) {
        entity->tick(delta_time);
    }
}
