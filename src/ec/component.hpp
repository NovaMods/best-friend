#pragma once

namespace nova::ec {
    // Some basic component definitions for Nova

    /*!
     * \brief Interface for your custom components
     *
     * This is similar to Unity's MonoBehaviour, NOT to UE4 ActorComponents
     *
     * When you make a class that implements Component, all its constructors _must_ take the owning entity as their
     * first parameter.
     */
    class Component {
    public:
        Component() = default;

        Component(const Component& other) = delete;
        Component& operator=(const Component& other) = delete;

        Component(Component&& old) noexcept = delete;
        Component& operator=(Component&& old) noexcept = delete;

        virtual ~Component() = default;

        /*!
         * \brief Initialize this component after the entity it belongs to is fully constructed
         *
         * Should be used to cache references to other component, call methods on other components, etc
         */
        virtual void begin_play() {}

        /*!
         * \brief Perform whatever per-frame work that this component performs
         */
        virtual void tick(double /* delta_time */) {}
    };

    /*!
     * \brief Component type that allows you to have a polymorphic component
     *
     * This allows one to write OOP code for your gameplay engine, while still allowing engine systems to use raw ECS
     */
    struct PolymorphicComponent {
       Component* component;
    };
} // namespace nova::ec
