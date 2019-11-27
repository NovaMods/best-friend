#pragma once

#include <memory>

#include <nuklear.h>

#include "panel.hpp"

/*!
 * \brief UI panel that allows the user to select a train
 */
class TrainSelectionPanel final : public Panel {
public:
	explicit TrainSelectionPanel(std::shared_ptr<nk_context> ctx);
	
	void draw() override;

private:
	std::shared_ptr<nk_context> ctx;
};