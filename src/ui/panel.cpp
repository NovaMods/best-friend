#include "panel.hpp"

Panel::Panel(nova::ec::Entity* owner) : Component(owner) {}

void Panel::tick(const double /* delta_time */) { draw(); }
