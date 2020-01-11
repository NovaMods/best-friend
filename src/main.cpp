#include <iostream>

#include <nova_renderer/nova_renderer.hpp>

#include "nova_renderer/util/logger.hpp"

#include "ec/entity.hpp"
#include "renderer/nk_backend.hpp"
#include "ui/train_selection_panel.hpp"
#include "world/world.hpp"

using namespace nova;
using namespace bf;
using namespace renderer;
using namespace filesystem;
using namespace ec;

void setup_logger() {
    // TODO: Nova needs a better way to handle this
    auto error_log = std::make_shared<std::ofstream>();
    error_log->open("test_error_log.log");
    auto test_log = std::make_shared<std::ofstream>("test_log.log");
    auto& log = Logger::instance;
    log.add_log_handler(TRACE, [test_log](auto msg) {
        std::cout << "TRACE: " << msg.c_str() << std::endl;
        *test_log << "TRACE: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(DEBUG, [test_log](auto msg) {
        std::cout << "DEBUG: " << msg.c_str() << std::endl;
        *test_log << "DEBUG: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(INFO, [test_log](auto msg) {
        std::cout << "INFO: " << msg.c_str() << std::endl;
        *test_log << "INFO: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(WARN, [test_log](auto msg) {
        std::cerr << "WARN: " << msg.c_str() << std::endl;
        *test_log << "WARN: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(ERROR, [test_log, error_log](auto msg) {
        std::cerr << "ERROR: " << msg.c_str() << std::endl;
        *error_log << "ERROR: " << msg.c_str() << std::endl << std::flush;
        *test_log << "ERROR: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(FATAL, [test_log, error_log](auto msg) {
        std::cerr << "FATAL: " << msg.c_str() << std::endl;
        *error_log << "FATAL: " << msg.c_str() << std::endl << std::flush;
        *test_log << "FATAL: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(MAX_LEVEL, [test_log, error_log](auto msg) {
        std::cerr << "MAX_LEVEL: " << msg.c_str() << std::endl;
        *error_log << "MAX_LEVEL: " << msg.c_str() << std::endl << std::flush;
        *test_log << "MAX_LEVEL: " << msg.c_str() << std::endl;
    });
}

int main(int argc, const char** argv) {
    setup_logger();

    NovaSettings settings;
#if NOVA_DEBUG
    settings.debug.enabled = true;
    settings.debug.enable_validation_layers = true;
#endif

    settings.window.title = "Best Friend Train Viewer";
    settings.window.width = 640;
    settings.window.height = 480;

#if WIN32
    settings.api = GraphicsApi::D3D12;
#else
    settings.api = GraphicsApi::Vulkan;
#endif

    VirtualFilesystem::get_instance()->add_resource_root(BEST_FRIEND_DATA_DIR);

    NovaRenderer renderer(settings);

    renderer.set_num_meshes(32); // Best guess, should fix when we know more

    std::unique_ptr<World> world = std::make_unique<World>();

    auto* nuklear_device = renderer.set_ui_renderpass(std::make_unique<NuklearDevice>(renderer),
                                                      NuklearDevice::get_create_info());

    renderer.load_renderpack("Simple");

    // Instantiate the basic entities
    // TODO: Make something more better
    auto* train_selection_entity = new Entity;
    train_selection_entity->add_component<TrainSelectionPanel>(nuklear_device->get_context());
    world->add_entity(train_selection_entity);

    const auto& window = renderer.get_window();

    auto frame_start_time = static_cast<double>(std::clock());
    auto frame_end_time = static_cast<double>(std::clock());
    auto last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;

    // Number of frames since program start
    uint64_t frame_counter = 0;

    while(!window.should_close()) {
        // Main loop!

        frame_start_time = static_cast<double>(std::clock());

        if(frame_counter % 100 == 0) {
            NOVA_LOG(INFO) << "Frame " << frame_counter << " took " << last_frame_duration << "ms\n";
        }

        world->tick(last_frame_duration);

        renderer.execute_frame();

        frame_end_time = static_cast<double>(std::clock());
        last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;
        frame_counter++;
    }

    return 0;
}
