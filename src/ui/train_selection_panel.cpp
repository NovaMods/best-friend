#include "train_selection_panel.hpp"

namespace nova::bf {
    TrainSelectionPanel::TrainSelectionPanel(ec::Entity* owner, nk_context* ctx) : Panel(owner), ctx(ctx) {}

    void TrainSelectionPanel::draw() {
        const bool is_train_selected = !last_loaded_train.is_empty();

        /*if(nk_begin(ctx,
                    "Train Selection",
                    {5, 5, 220, 200},
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE)) {
            nk_layout_row_dynamic(ctx, 0, 1);

            // Display the currently selected train file
            // Button to select a different train file, should open an OS-native file picker
            // Maybe some import options for edge smoothing? These should be cached, as should all generated data, so that loading a train
            // file can be very fast

            nk_text(ctx, selected_train_label.data(), 0, 0);
        }
        nk_end(ctx);*/

        const auto selected_train_label = [&]() -> rx::string {
            if(is_train_selected) {
                return rx::string::format("Selected train file: %s", last_loaded_train);

            } else {
                return "Please select a train";
            }
        }();

        if(nk_begin(ctx,
                    "Train Selection",
                    {5, 5, 425, 200},
                    NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE)) {

            // fixed widget pixel width
            nk_layout_row_static(ctx, 0, 400, 1);

            nk_label(ctx, selected_train_label.data(), NK_TEXT_LEFT);

            if(nk_button_label(ctx, "Choose train file")) {
                // event handling
            }
        }
        nk_end(ctx);
    }
} // namespace nova::bf
