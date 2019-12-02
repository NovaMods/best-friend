#pragma once

#include <memory>

#include <nuklear.h>

#include "panel.hpp"
#include <string>

/*!
 * \brief UI panel that allows the user to select a train
 */
class TrainSelectionPanel final : public Panel {
public:
    explicit TrainSelectionPanel(nova::ec::Entity* owner, std::shared_ptr<nk_context> ctx);

    void draw() override;

private:
    std::shared_ptr<nk_context> ctx;

    std::string last_loaded_train;
};