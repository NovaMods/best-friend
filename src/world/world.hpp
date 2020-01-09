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
     * \param entity The entity to add to the world
     */
    void add_entity(nova::ec::Entity* entity);

    /*!
     * \brief Removes an entity from the world
     *
     * This method does _not_ deallocate the entity. The caller must deallocate the entity
     *
     * \param entity The entity to remove from the world
     */
    void remove_entity(nova::ec::Entity* entity);

    /*!
     * \brief Ticks the world and all the entities in it
     * 
     * \param delta_time The amount of time since the world was last ticked
     */
    void tick(double delta_time);

private:
    std::vector<nova::ec::Entity*> entities;
};
