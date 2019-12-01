#pragma once

#include <memory>

#include <nuklear.h>

class PanelManager {
private:
    std::shared_ptr<nk_context> ctx;
};