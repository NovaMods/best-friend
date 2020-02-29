#include <bve.hpp>
#include <entt/entt.hpp>
#include <nova_renderer/nova_renderer.hpp>

#include "ec/system.hpp"
#include "ec/transform.hpp"
#include "loading/data_loader.hpp"
#include "renderer/camera_component.hpp"
#include "renderer/nk_backend.hpp"
#include "renderer/train_rendering.hpp"
#include "ui/train_selection_panel.hpp"
using namespace nova;
using namespace bf;
using namespace renderer;
using namespace filesystem;
using namespace ec;

RX_LOG("BestFriend", logger);

void tick_polymorphic_components(entt::registry& registry, const double delta_time) {
    auto view = registry.view<PolymorphicComponent>();

    for(auto entity : view) {
        auto& component = view.get<PolymorphicComponent>(entity);
        component.component->tick(delta_time);
    }
}

int main(int argc, const char** argv) {
    init_rex();

    NovaSettings settings;
#if NOVA_DEBUG
    settings.debug.enabled = true;
    settings.debug.enable_validation_layers = true;
    settings.debug.enable_gpu_based_validation = true;
#endif

    settings.window.title = "Best Friend Train Viewer";
    settings.window.width = 640;
    settings.window.height = 480;

    VirtualFilesystem::get_instance()->add_resource_root(BEST_FRIEND_DATA_DIR);

    // TODO: Set BVE panic handlers and whatnot
    bve::bve_init();

    NovaRenderer renderer{settings};

    renderer.set_num_meshes(32); // Best guess, should fix when we know more

    auto* nuklear_device = renderer.create_ui_renderpass<NuklearDevice>(&renderer);

    renderer.load_renderpack("Simple");

    entt::registry registry;

    DataLoader loader{registry, renderer};

    // Instantiate the basic entities
    // TODO: Make something more better
    const auto train_selection_entity = registry.create();
    registry.assign<PolymorphicComponent>(train_selection_entity, new ui::TrainSelectionPanel{nuklear_device->get_context()});

    CameraCreateInfo create_info = {};
    create_info.name = "BestFriendCamera";

    const auto camera = registry.create();
    registry.assign<Transform>(camera);
    registry.assign<CameraComponent>(camera, renderer.create_camera(create_info));

    const auto& window = renderer.get_window();

    auto frame_start_time = static_cast<double>(clock());
    auto frame_end_time = static_cast<double>(clock());
    auto last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;

    // Number of frames since program start
    uint64_t frame_counter = 0;

    while(!window.should_close()) {
        // Main loop!

        frame_start_time = static_cast<double>(clock());

        if(frame_counter % 100 == 0) {
            logger(rx::log::level::k_info, "Frame %u took %fms", frame_counter, last_frame_duration * 1000);
        }

        window.poll_input();

        tick_polymorphic_components(registry, last_frame_duration);
        update_camera_positions(registry);

        update_renderable_transforms(registry, renderer);

        renderer.execute_frame();

        frame_end_time = static_cast<double>(clock());
        last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;
        frame_counter++;
    }

    logger(rx::log::level::k_warning, "REMAIN INDOORS");

    return 0;
}
