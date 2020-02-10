#include "train_selection_panel.hpp"

namespace nova::bf {
    TrainSelectionPanel::TrainSelectionPanel(ec::Entity* owner, nk_context* ctx) : Panel(owner), ctx(ctx) {}

    void TrainSelectionPanel::draw() {
        const bool is_train_selected = !last_loaded_train.is_empty();

        if(nk_begin(ctx,
                    "Train Selection",
                    {50, 50, 220, 200},
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE)) {
            nk_layout_row_static(ctx, 30, 80, 1);

            // Display the currently selected train file
            // Button to select a different train file, should open an OS-native file picker
            // Maybe some import options for edge smoothing? These should be cached, as should all generated data, so that loading a train
            // file can be very fast

            const auto selected_train_label = [&]() -> rx::string {
                if(is_train_selected) {
                    return rx::string::format("Selected train file: %s", last_loaded_train);

                } else {
                    return "Please select a train";
                }
            }();

            nk_label(ctx, selected_train_label.data(), 0);
        }
        nk_end(ctx);
    }
} // namespace nova::bf
