#include "train_selection_panel.hpp"

#include <utility>

TrainSelectionPanel::TrainSelectionPanel(std::shared_ptr<nk_context> ctx) : ctx(std::move(ctx)) {}

void TrainSelectionPanel::draw() {
    if(nk_begin(ctx.get(),
                "Train Selection",
                {50, 50, 220, 200},
                NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE)) {
        nk_layout_row_static(ctx.get(), 30, 80, 1);
    }
}
