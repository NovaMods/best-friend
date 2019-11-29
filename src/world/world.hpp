#pragma once
#include <vector>

namespace nova::ec {
    class Entity;
}

/*!
 * \brief The world that currently exists
 *
 * When you create an object through some mechanism you add it to the world. Everything in the world will tick, no
 * matter what
 *
 * There may be different worlds for the editor vs game vs preview
 */
class World {
public:
    /*!
     * \brief Adds a new entity to the world
     *
     * Entities begin ticking the frame after they're added to the world. This is so entities may be added from
     * multiple threads, facilitating async loading
     *
     * \param entity The entitiy to add tot he world
     */
    void add_entity(nova::ec::Entity* entity);

private:
    std::vector<nova::ec::Entity*> Entities;
};