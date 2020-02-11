#include "ui_events.hpp"

namespace nova::bf {
    UiEventBus UiEventBus::instance;

    UiEventBus& UiEventBus::get_instance() { return instance; }

    entt::dispatcher& UiEventBus::get_dispatcher() { return event_dispatcher; }
} // namespace nova::bf
