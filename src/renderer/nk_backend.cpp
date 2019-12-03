#include "nk_backend.hpp"

#include <nova_renderer/nova_renderer.hpp>

#define NK_IMPLEMENTATION
#include <nuklear.h>

#include "../util/constants.hpp"

using namespace nova::renderer;
using namespace shaderpack;
using namespace rhi;

struct UiDrawParams {
    glm::mat4 projection;
};

struct NkVertex {
    glm::vec2 position;
    glm::vec2 uv;
    glm::u8vec4 color;
};

NuklearDevice::NuklearDevice(NovaRenderer& renderer)
    : renderer(renderer), mesh(renderer.create_procedural_mesh(MAX_VERTEX_BUFFER_SIZE, MAX_INDEX_BUFFER_SIZE)) {

    // TODO: Some way to validate that this pass exists in the loaded shaderpack
    const FullMaterialPassName& ui_full_material_pass_name = {"UI", "Color"};
    StaticMeshRenderableData ui_renderable_data = {};
    ui_renderable_data.mesh = mesh.get_key();
    ui_renderable_id = renderer.add_renderable_for_material(ui_full_material_pass_name, ui_renderable_data);

    const auto& window = renderer.get_engine()->get_window();
    window.register_key_callback([&](const auto& key) {
        std::lock_guard l(key_buffer_mutex);
        keys.push_back(key);
    });
}

void NuklearDevice::begin_frame() {
    nk_input_begin(ctx);

    for(const auto key : keys) {
        nk_input_unicode(ctx, key);
    }

    nk_input_end(ctx);
}

void NuklearDevice::render() {

}
