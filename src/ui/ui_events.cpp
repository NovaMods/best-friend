#include "ui_events.hpp"

#include "../util/constants.hpp"

namespace nova::bf {
    rx::global<entt::dispatcher> g_ui_event_bus{BEST_FRIEND_GLOBALS_GROUP, "UiEventBus"};
} // namespace nova::bf
