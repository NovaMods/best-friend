#include "train_selection_panel.hpp"

#include <rx/core/log.h>

#include "../loading/train_loading.hpp"
#include "ui_events.hpp"

RX_LOG("TrainPanel", logger);

namespace nova::bf {
    TrainSelectionPanel::TrainSelectionPanel(ec::Entity* owner, nk_context* ctx) : Panel(owner), ctx(ctx) {}

    void TrainSelectionPanel::draw() {
        const bool is_train_selected = !last_loaded_train.is_empty();

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
            nk_layout_row_dynamic(ctx, 0, 1);

            nk_label(ctx, selected_train_label.data(), NK_TEXT_LEFT);

            if(nk_button_label(ctx, "Choose train file")) {
                logger(rx::log::level::k_verbose, "Clicked choose train button");

                last_loaded_train = BEST_FRIEND_DATA_DIR "/trains/R46 2014 (8 Car)/Cars/Body/BodyA.b3d";
                g_ui_event_bus->trigger<LoadTrainEvent>(last_loaded_train);
            }
        }
        nk_end(ctx);
    }
} // namespace nova::bf
