#pragma once
#include <iostream>
#include <vector>

class World;

namespace nova::ec {

    class Component;

    /*!
     * \brief An entity in a game
     *
     * In Nova, entities have a tick method which ticks all their components. I've intentionally done this instead of
     * a true ECS because ECS is cumbersome for gameplay code
     *
     * You're expected to subclass this for your entites. The subclass's constructor should add all the components you
     * want to have. A future version of this library will be able to generate entity classes from a config file, but
     * we're not there yet
     */
    class Entity {
    public:
        /*!
         * \brief The world that this entity belongs to
         */
        World* world;

        // TODO: Each entity should have its own component allocator, so it can allocate its components near each other
        // in RAM. This will be most useful after I get the codegen working, because then I'll know at compile time
        // how much space each entity needs, and thus how much space the allocator should have
        Entity();

        Entity(const Entity& other) = delete;
        Entity& operator=(const Entity& other) = delete;

        virtual ~Entity() = default;

        uint64_t get_id() const;

        /*!
         * \brief Ticks this entity and also all its components
         */
        void tick(double delta_time);

        /*!
         * \brief Adds a new component of the specified type to this entity, forwarding all parameters to the
         * component's constructor
         *
         * This method first checks if this entity already has a component of the desired type. If so, it returns
         * nullptr. If not, it allocates a new component of the desired type and adds it to this entity's components
         * list
         */
        template <typename ComponentType, typename... Args>
        ComponentType* add_component(Args... args) {
            // TODO: Smarter allocation
            // TODO: compile-time check that the first argument to the constructor is the owning entity
            auto* old_component = get_component<ComponentType>();
            if(old_component) {
                std::cerr << "Entity already has a component of this type\n";
                return nullptr;
            }

            ComponentType* new_component = new ComponentType(this, std::forward(args...));
            components.push_back(new_component);

            return new_component;
        }

        /*!
         * \brief Retrieves the component of the specified type, if it exists. If not, returns nullptr
         */
        template <typename ComponentType>
        ComponentType* get_component() {
            // TODO: Figure out what run-time type info we need to store, and store it
            return nullptr;
        }

    private:
        static uint64_t next_id = 0;
        uint64_t id;

        /*!
         * \brief All the components attached to this entity
         */
        std::vector<Component*> components;
    };
} // namespace nova::ec
