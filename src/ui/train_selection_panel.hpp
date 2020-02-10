#pragma once

#include <nuklear.h>
#include <rx/core/string.h>

#include "panel.hpp"

namespace nova::bf {
    /*!
     * \brief UI panel that allows the user to select a train
     */
    class TrainSelectionPanel final : public Panel {
    public:
        explicit TrainSelectionPanel(ec::Entity* owner, nk_context* ctx);

        void draw() override;

    private:
        nk_context* ctx;

        rx::string last_loaded_train;
    };
} // namespace nova::bf