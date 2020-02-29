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
     */
    extern rx::global<entt::dispatcher> g_ui_event_bus;
} // namespace nova::bf
