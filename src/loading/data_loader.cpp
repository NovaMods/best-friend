#include "data_loader.hpp"

#include <minitrace.h>
#include <nova_renderer/nova_renderer.hpp>
#include <rx/core/log.h>

#include "../ec/transform.hpp"
#include "../renderer/train_rendering.hpp"
#include "../ui/ui_events.hpp"
#include "bve_wrapper.hpp"
#include "train_loading.hpp"

namespace nova::bf {
    RX_LOG("DataLoader", logger);

    struct BestFriendTrainVertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };

    inline glm::vec3 to_vec3(const bve::Vector3<float>& vec) { return {vec.x, vec.y, vec.z}; }

    inline glm::vec2 to_vec2(const bve::Vector2<float>& vec) { return {vec.x, vec.y}; }

    DataLoader::DataLoader(entt::registry& world, renderer::NovaRenderer& renderer_in) : registry(world), renderer(renderer_in) {
        g_ui_event_bus->sink<LoadTrainEvent>().connect<&DataLoader::load_train>(this);
    }

    void DataLoader::load_train(const LoadTrainEvent& event) const {
        MTR_SCOPE("DataLoader::load_train", "All");

        // TODO: Local allocator for each train

        const renderer::FullMaterialPassName pass_name{"Train", "main"};

        auto train = load_train_mesh(event.filepath);
        if(train) {
            MTR_SCOPE("DataLoader::load_train", "SendTrainToGpu");
            logger->verbose("This chimera didn't explode");

            auto train_entity = registry.create();
            registry.assign<Transform>(train_entity);
            registry.assign<HierarchyMember>(train_entity);

            const auto train_meshes = train->meshes;

            for(uint32_t i = 0; i < train_meshes.count; i++) {
                const auto train_mesh = train_meshes.ptr[i];

                logger->verbose("Processing a mesh with %u vertices and %u indices", train_mesh.vertices.count, train_mesh.indices.count);

                auto& allocator = rx::memory::system_allocator::instance();
                rx::vector<BestFriendTrainVertex> vertex_data{allocator};
                vertex_data.reserve(train_mesh.vertices.count);
                for(uint32_t v = 0; v < train_mesh.vertices.count; v++) {
                    const auto& cur_vert = train_mesh.vertices.ptr[v];
                    vertex_data.emplace_back(to_vec3(cur_vert.position), to_vec3(cur_vert.normal), to_vec2(cur_vert.coord));
                }

                rx::vector<uint32_t> indices{allocator};
                indices.reserve(train_mesh.indices.count);
                for(uint32_t idx = 0; idx < train_mesh.indices.count; idx++) {
                    const auto& index = train_mesh.indices.ptr[idx];
                    indices.emplace_back(static_cast<uint32_t>(index));
                }

                renderer::MeshData mesh_data{3,
                                             static_cast<uint32_t>(train_mesh.indices.count),
                                             vertex_data.data(),
                                             vertex_data.size() * sizeof(BestFriendTrainVertex),
                                             indices.data(),
                                             indices.size() * sizeof(uint32_t)};

                const auto mesh = renderer.create_mesh(mesh_data);
                logger->verbose("Added mesh %u", mesh);

                renderer::StaticMeshRenderableCreateInfo renderable_data{};
                renderable_data.mesh = mesh;
                renderable_data.scale = glm::vec3{0.01f};

                const auto renderable = renderer.add_renderable_for_material(pass_name, renderable_data);
                logger->verbose("Added renderable %u", renderable);

                const auto material = renderer.create_material<TrainMaterial>();
                material.second->color_texture = 0; // TODO: Load the textures like a real boy

                auto train_mesh_entity = registry.create();
                registry.assign<Transform>(train_mesh_entity);
                registry.assign<RenderableComponent>(train_mesh_entity, true, renderable, material.first);
                registry.assign<HierarchyMember>(train_mesh_entity, train_entity, rx::vector<entt::entity>{});

                auto& train_hierarchy = registry.get<HierarchyMember>(train_entity);
                train_hierarchy.children.emplace_back(train_mesh_entity);
            }

            g_bve->delete_parsed_static_object(rx::utility::move(*train));
        }
    }
} // namespace nova::bf
