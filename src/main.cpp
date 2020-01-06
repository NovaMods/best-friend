#include <iostream>

#include <nova_renderer/nova_renderer.hpp>

#include "nova_renderer/util/logger.hpp"

#include "ec/entity.hpp"
#include "renderer/nk_backend.hpp"
#include "ui/train_selection_panel.hpp"
#include "world/world.hpp"

void setup_logger() {
    // TODO: Nova needs a better way to handle this
    auto error_log = std::make_shared<std::ofstream>();
    error_log->open("test_error_log.log");
    auto test_log = std::make_shared<std::ofstream>("test_log.log");
    auto& log = nova::renderer::Logger::instance;
    log.add_log_handler(nova::renderer::TRACE, [test_log](auto msg) {
        std::cout << "TRACE: " << msg.c_str() << std::endl;
        *test_log << "TRACE: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(nova::renderer::DEBUG, [test_log](auto msg) {
        std::cout << "DEBUG: " << msg.c_str() << std::endl;
        *test_log << "DEBUG: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(nova::renderer::INFO, [test_log](auto msg) {
        std::cout << "INFO: " << msg.c_str() << std::endl;
        *test_log << "INFO: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(nova::renderer::WARN, [test_log](auto msg) {
        std::cerr << "WARN: " << msg.c_str() << std::endl;
        *test_log << "WARN: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(nova::renderer::ERROR, [test_log, error_log](auto msg) {
        std::cerr << "ERROR: " << msg.c_str() << std::endl;
        *error_log << "ERROR: " << msg.c_str() << std::endl << std::flush;
        *test_log << "ERROR: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(nova::renderer::FATAL, [test_log, error_log](auto msg) {
        std::cerr << "FATAL: " << msg.c_str() << std::endl;
        *error_log << "FATAL: " << msg.c_str() << std::endl << std::flush;
        *test_log << "FATAL: " << msg.c_str() << std::endl;
    });
    log.add_log_handler(nova::renderer::MAX_LEVEL, [test_log, error_log](auto msg) {
        std::cerr << "MAX_LEVEL: " << msg.c_str() << std::endl;
        *error_log << "MAX_LEVEL: " << msg.c_str() << std::endl << std::flush;
        *test_log << "MAX_LEVEL: " << msg.c_str() << std::endl;
    });
}

int main(int argc, const char** argv) {
    setup_logger();

    nova::renderer::NovaSettings settings;
#ifndef NDEBUG
    settings.debug.enabled = true;
#endif

    settings.window.title = "Best Friend Train Viewer";
    settings.window.width = 640;
    settings.window.height = 480;

#if WIN32
    settings.api = nova::renderer::GraphicsApi::Vulkan;
#else
    settings.api = nova::renderer::GraphicsApi::Vulkan;
#endif

    nova::filesystem::VirtualFilesystem::get_instance()->add_resource_root(BEST_FRIEND_DATA_DIR);

    nova::renderer::NovaRenderer renderer(settings);

    renderer.set_num_meshes(32); // Best guess, should fix when we know more

    std::unique_ptr<World> world = std::make_unique<World>();
    const std::shared_ptr<nova::bf::NuklearDevice> nuklear_device = std::make_shared<nova::bf::NuklearDevice>(renderer);

    // TODO: Load the renderpack _after_ registering the UI render pass
    renderer.set_ui_renderpass(nuklear_device);

    // Instantiate the basic entities
    // TODO: Make something more better
    nova::ec::Entity* train_selection_entity = new nova::ec::Entity();
    train_selection_entity->add_component<nova::bf::TrainSelectionPanel>(nuklear_device->get_context());
    world->add_entity(train_selection_entity);

    const auto& window = renderer.get_window();

    auto frame_start_time = static_cast<double>(std::clock());
    auto frame_end_time = static_cast<double>(std::clock());
    auto last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;

    // Number of frames since program start
    uint64_t frame_counter = 0;

    while(!window->should_close()) {
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
