#pragma once

#include <memory>
#include <string>

#include <nuklear.h>

#include "panel.hpp"

namespace nova::bf {
    /*!
     * \brief UI panel that allows the user to select a train
     */
    class TrainSelectionPanel final : public Panel {
    public:
        explicit TrainSelectionPanel(ec::Entity* owner, std::shared_ptr<nk_context> ctx);

        void draw() override;

    private:
        std::shared_ptr<nk_context> ctx;

        std::string last_loaded_train;
    };
} // namespace nova::bf