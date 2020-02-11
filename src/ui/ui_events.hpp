#pragma once

#include <entt/signal/dispatcher.hpp>
#include <rx/core/string.h>

namespace nova::bf {
    struct LoadTrainEvent {
        rx::string filepath;
    };

    /*!
     * \brief Event bus for all UI events
     *
     * There's one dispatcher. You and your friends can register events subscribers, me and my friends can post events, it's totally great
     *
     * Yes it's a singleton hecc off
     */
    class UiEventBus {
    public:
        static UiEventBus& get_instance();

        entt::dispatcher& get_dispatcher();

    private:
        static UiEventBus instance;

        entt::dispatcher event_dispatcher;
    };
} // namespace nova::bf
