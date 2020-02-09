#include <nova_renderer/nova_renderer.hpp>

#include "ec/entity.hpp"
#include "renderer/nk_backend.hpp"
#include "ui/train_selection_panel.hpp"
#include "world/world.hpp"

using namespace nova;
using namespace bf;
using namespace renderer;
using namespace filesystem;
using namespace ec;

RX_LOG("BestFriend", logger);

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

    NovaRenderer renderer(settings);

    renderer.set_num_meshes(32); // Best guess, should fix when we know more

    auto* allocator = &rx::memory::g_system_allocator;

    auto* world = allocator->create<World>();

    auto* nuklear_device = allocator->create<NuklearDevice>(renderer);

    renderer.set_ui_renderpass(nuklear_device, NuklearDevice::get_create_info());

    renderer.load_renderpack("Simple");

    // Instantiate the basic entities
    // TODO: Make something more better
    auto* train_selection_entity = new Entity;
    train_selection_entity->add_component<TrainSelectionPanel>(nuklear_device->get_context());
    world->add_entity(train_selection_entity);

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
            logger(rx::log::level::k_info, "Frame %u took %ums", frame_counter, last_frame_duration);
        }

        world->tick(last_frame_duration);

        renderer.execute_frame();

        frame_end_time = static_cast<double>(clock());
        last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;
        frame_counter++;
    }

    return 0;
}
