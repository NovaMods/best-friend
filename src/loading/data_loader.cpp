#include "data_loader.hpp"

#include <nova_renderer/nova_renderer.hpp>
#include <rx/core/log.h>

#include "../ec/entity.hpp"
#include "../ui/ui_events.hpp"
#include "../world/world.hpp"
#include "train_loading.hpp"
#include "rx/core/profiler.h"

namespace nova::bf {
    RX_LOG("DataLoader", logger);

    inline glm::vec3 to_vec3(const bve::Vector3<float>& vec) { return {vec.x, vec.y, vec.z}; }

    inline glm::vec2 to_vec2(const bve::Vector2<float>& vec) { return {vec.x, vec.y}; }

    DataLoader::DataLoader(World& world, renderer::NovaRenderer& renderer) : world(world), renderer(renderer) {
        ui_event_bus->sink<LoadTrainEvent>().connect<&DataLoader::load_train>(this);
    }

    void DataLoader::load_train(const LoadTrainEvent& event) {
        rx::profiler::cpu_sample load_mesh_sample{"LoadMesh"};

        const renderer::FullMaterialPassName pass_name{"Train", "main"};

        const auto train = load_train_mesh(event.filepath);
        if(train) {
            logger(rx::log::level::k_verbose, "This chimera didn't explode");

            auto* train_entity = new ec::Entity;

            const auto train_meshes = train->meshes;

            for(uint32_t i = 0; i < train_meshes.count; i++) {
                rx::profiler::cpu_sample upload_mesh_sample{rx::string::format("UploadMeshPart%u", i).data()};
                const auto train_mesh = train_meshes.ptr[i];

                logger(rx::log::level::k_verbose,
                       "Processing a mesh with %u vertices and %u indices",
                       train_mesh.vertices.count,
                       train_mesh.indices.count);

                renderer::MeshData mesh_data{};
                mesh_data.vertex_data.reserve(train_mesh.vertices.count);
                for(uint32_t vert_idx = 0; vert_idx < train_mesh.vertices.count; vert_idx++) {
                    const auto vertex = train_mesh.vertices.ptr[vert_idx];
                    mesh_data.vertex_data.emplace_back(
                        renderer::FullVertex{to_vec3(vertex.position), to_vec3(vertex.normal), {}, {}, {}, {}, {}});
                }

                mesh_data.indices.reserve(train_mesh.indices.count);
                for(uint32_t idx = 0; idx < train_mesh.indices.count; idx++) {
                    mesh_data.indices.emplace_back(static_cast<uint32_t>(train_mesh.indices.ptr[idx]));
                }

                const auto mesh = renderer.create_mesh(mesh_data);
                logger(rx::log::level::k_verbose, "Added mesh %u", mesh);

                renderer::StaticMeshRenderableData renderable_data{};
                renderable_data.mesh = mesh;
                renderable_data.initial_scale = {0.01};

                const auto renderable = renderer.add_renderable_for_material(pass_name, renderable_data);
                logger(rx::log::level::k_verbose, "Added renderable %u", renderable);
            }

            world.add_entity(train_entity);
        }
    }
} // namespace nova::bf
