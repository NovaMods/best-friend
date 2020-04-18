#include <bve.hpp>
#include <entt/entt.hpp>
#include <nova_renderer/nova_renderer.hpp>

#include "ec/system.hpp"
#include "ec/transform.hpp"
#include "input/input.hpp"
#include "loading/data_loader.hpp"
#include "renderer/camera_component.hpp"
#include "renderer/nk_backend.hpp"
#include "renderer/train_rendering.hpp"
#include "rx/core/memory/buddy_allocator.h"
#include "ui/train_selection_panel.hpp"
#include "util/constants.hpp"

using namespace nova;
using namespace bf;
using namespace renderer;
using namespace filesystem;
using namespace ec;

rx::global_group g_best_friend_group{BEST_FRIEND_GLOBALS_GROUP};

RX_LOG("BestFriend", logger);

struct CubeVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

void tick_polymorphic_components(entt::registry& registry, const double delta_time) {
    MTR_SCOPE("BestFriend", "tick_polymorphic_components");
    const auto& view = registry.view<PolymorphicComponent>();

    for(auto entity : view) {
        auto& component = view.get<PolymorphicComponent>(entity);
        component.component->tick(delta_time);
    }
}

static void create_debug_cube(NovaRenderer& renderer, entt::registry& registry) {
    const rx::vector<CubeVertex> cube_vertices = rx::array{
        // Front
        CubeVertex{{-0.5, 0.5, -0.5}, {0, 0, -1}, {}},
        CubeVertex{{0.5, 0.5, -0.5}, {0, 0, -1}, {}},
        CubeVertex{{0.5, -0.5, -0.5}, {0, 0, -1}, {}},
        CubeVertex{{-0.5, -0.5, -0.5}, {0, 0, -1}, {}},

        // Right
        CubeVertex{{0.5, 0.5, -0.5}, {1, 0, 0}, {}},
        CubeVertex{{0.5, 0.5, 0.5}, {1, 0, 0}, {}},
        CubeVertex{{0.5, -0.5, 0.5}, {1, 0, 0}, {}},
        CubeVertex{{0.5, -0.5, -0.5}, {1, 0, 0}, {}},

        // Back
        CubeVertex{{0.5, 0.5, 0.5}, {0, 0, 1}, {}},
        CubeVertex{{0.5, -0.5, 0.5}, {0, 0, 1}, {}},
        CubeVertex{{-0.5, -0.5, 0.5}, {0, 0, 1}, {}},
        CubeVertex{{-0.5, 0.5, 0.5}, {0, 0, 1}, {}},

        // Left
        CubeVertex{{-0.5, 0.5, 0.5}, {-1, 0, 0}, {}},
        CubeVertex{{-0.5, 0.5, -0.5}, {-1, 0, 0}, {}},
        CubeVertex{{-0.5, -0.5, -0.5}, {-1, 0, 0}, {}},
        CubeVertex{{-0.5, -0.5, 0.5}, {-1, 0, 0}, {}},

        // Top
        CubeVertex{{-0.5, -0.5, -0.5}, {0, -1, 0}, {}},
        CubeVertex{{0.5, -0.5, -0.5}, {0, -1, 0}, {}},
        CubeVertex{{0.5, -0.5, 0.5}, {0, -1, 0}, {}},
        CubeVertex{{-0.5, -0.5, 0.5}, {0, -1, 0}, {}},

        // Bottom
        CubeVertex{{-0.5, 0.5, -0.5}, {0, 1, 0}, {}},
        CubeVertex{{0.5, 0.5, -0.5}, {0, 1, 0}, {}},
        CubeVertex{{0.5, 0.5, 0.5}, {0, 1, 0}, {}},
        CubeVertex{{-0.5, 0.5, 0.5}, {0, 1, 0}, {}},

    };

    const auto indices = rx::array{0,  1,  2,  1,  2,  3,  4,  5,  6,  5,  6,  7,  8,  9,  10, 9,  10, 11,
                                   12, 13, 14, 13, 14, 15, 16, 17, 18, 17, 18, 19, 20, 21, 22, 21, 22, 23};

    const MeshData mesh_data{3,
                             indices.size(),
                             cube_vertices.data(),
                             cube_vertices.size() * sizeof(CubeVertex),
                             indices.data(),
                             indices.size() * sizeof(uint32_t)};
    const auto cube_mesh = renderer.create_mesh(mesh_data);

    StaticMeshRenderableCreateInfo cube_renderable_create_info{};
    cube_renderable_create_info.mesh = cube_mesh;

    const renderer::FullMaterialPassName pass_name{"Train", "main"};
    const auto cube_renderable = renderer.add_renderable_for_material(pass_name, cube_renderable_create_info);

    const auto cube_material = renderer.create_material<TrainMaterial>();
    cube_material.second->color_texture = 0;

    const auto debug_cube = registry.create();
    registry.assign<Transform>(debug_cube);
    registry.assign<RenderableComponent>(debug_cube, true, cube_renderable, cube_material.first);
}

int main(int /* argc */, const char** /* argv */) {
    init_rex();

    logger->info("HELLO HUMAN");

    NovaSettings settings;
#ifdef NOVA_DEBUG
    settings.debug.enabled = true;
    settings.debug.enable_validation_layers = true;
    settings.debug.enable_gpu_based_validation = false;
    settings.debug.renderdoc.enabled = false;
#endif

    settings.window.title = "Best Friend Train Viewer";
    settings.window.width = 640;
    settings.window.height = 480;
    settings.api = Api::Vulkan;

    VirtualFilesystem::get_instance()->add_resource_root(BEST_FRIEND_DATA_DIR);

    // TODO: Set BVE panic handlers and whatnot
    bve::bve_init();

    constexpr size_t nova_memory_size = 64 << 20;

    auto& system_allocator = rx::memory::system_allocator::instance();
    auto* nova_mem = system_allocator.allocate(nova_memory_size);
    rx::memory::buddy_allocator nova_allocator{nova_mem, nova_memory_size};

    NovaRenderer renderer{nova_allocator, settings};

    renderer.set_num_meshes(64); // Best guess, should fix when we know more

    auto* nuklear_device = renderer.create_ui_renderpass<NuklearUiPass>(&renderer);

    renderer.load_renderpack("Simple");

    entt::registry registry;

    // Register callbacks to hook up input system to input-receiving component
    InputRouter router{registry, renderer.get_window()};

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
    registry.assign<RotateAroundPointCameraController>(camera);

    const auto& window = renderer.get_window();

    create_debug_cube(renderer, registry);

    auto frame_start_time = static_cast<double>(clock());
    auto frame_end_time = static_cast<double>(clock());
    auto last_frame_duration = (frame_end_time - frame_start_time) / static_cast<double>(CLOCKS_PER_SEC);

    // Number of frames since program start
    uint64_t frame_counter = 0;

    rx::log::flush();

    while(!window.should_close()) {
        MTR_SCOPE("BestFriend", "main_loop");
        // Main loop!

        if(frame_counter % 100 == 0) {
            logger->info("Frame %u took %fms\nStart time: %f\nEnd time: %f",
                         frame_counter,
                         last_frame_duration * 1000.0,
                         frame_start_time,
                         frame_end_time);
        }

        frame_start_time = static_cast<double>(clock());

        window.poll_input();

        tick_polymorphic_components(registry, last_frame_duration);

        router.update_rotating_entities(last_frame_duration);

        update_camera_positions(registry);

        update_renderable_transforms(registry, renderer);

        renderer.execute_frame();

        frame_end_time = static_cast<double>(clock());
        last_frame_duration = (frame_end_time - frame_start_time) / static_cast<double>(CLOCKS_PER_SEC);
        frame_counter++;
    }

    logger->warning("REMAIN INDOORS");

    return 0;
}
