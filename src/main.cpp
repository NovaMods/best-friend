#include <iostream>

#include <nova_renderer/nova_renderer.hpp>

#include "ec/entity.hpp"
#include "ui/train_selection_panel.hpp"
#include "world/world.hpp"
#include "renderer/nk_backend.hpp"

int main(int argc, const char** argv) {
    std::cout << "Hello, world!";

    nova::renderer::NovaSettings settings;
#ifndef NDEBUG
    settings.debug.enabled = true;
#endif

    settings.window.title = "Best Friend Train Viewer";
    settings.window.width = 640;
    settings.window.height = 480;

#if WIN32
    settings.api = nova::renderer::GraphicsApi::D3D12;
#else
    settings.api = nova::renderer::GraphicsApi::Vulkan;
#endif

    nova::renderer::NovaRenderer renderer(settings);

    renderer.set_num_meshes(32); // Best guess, should fix when we know more

    std::unique_ptr<World> world = std::make_unique<World>();

    std::shared_ptr<nova::bf::NuklearDevice> nuklear_device = std::make_shared<nova::bf::NuklearDevice>(renderer);

    // TODO: Load the renderpack _after_ registering the UI render pass
    renderer.set_ui_renderpass(nuklear_device);

    // Instantiate the basic entities
    // TODO: Make something more better
    nova::ec::Entity* train_selection_entity = new nova::ec::Entity();
    train_selection_entity->add_component<TrainSelectionPanel>(nuklear_device->get_context());
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
            std::cout << "Frame " << frame_counter << " took " << last_frame_duration << "ms\n";
        }

        world->tick(last_frame_duration);

        frame_end_time = static_cast<double>(std::clock());
        last_frame_duration = (frame_end_time - frame_start_time) / CLOCKS_PER_SEC;
        frame_counter++;
    }

    return 0;
}
