#include "panel.hpp"

Panel::Panel(nova::ec::Entity* owner_in) : Component(owner_in) {}

void Panel::tick(const double /* delta_time */) { draw(); }
