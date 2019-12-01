#include "train_selection_panel.hpp"

#include <utility>

TrainSelectionPanel::TrainSelectionPanel(nova::ec::Entity* owner, std::shared_ptr<nk_context> ctx) : Panel(owner), ctx(std::move(ctx)) {}

void TrainSelectionPanel::draw() {
    const bool is_train_selected = !last_loaded_train.empty();

    if(nk_begin(ctx.get(),
                "Train Selection",
                {50, 50, 220, 200},
                NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE)) {
        nk_layout_row_static(ctx.get(), 30, 80, 1);

        // Display the currently selected train file
        // Button to select a different train file, should open an OS-native file picker
        // Maybe some import options for edge smoothing? These should be cached, as should all generated data, so that loading a train file
        // can be very fast

        const auto selected_train_label = [&]() -> std::string {
            if(is_train_selected) {
                return "Selected train file: " + last_loaded_train;

            } else {
                return "Please select a train";
            }
        }();

        nk_label(ctx.get(), selected_train_label.c_str(), 0);
    }
}
