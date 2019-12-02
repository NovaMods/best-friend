#include "nk_backend.hpp"

#include <nova_renderer/nova_renderer.hpp>

#define NK_IMPLEMENTATION
#include <nuklear.h>

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

NuklearDevice::NuklearDevice(nova::renderer::NovaRenderer& renderer) : renderer(renderer) {
    pipeline = make_pipeline();

    RenderEngine* render_engine = renderer.get_engine();

    render_engine->create_buffer()
}

void NuklearDevice::render(nk_context* ctx) {
    RenderEngine* render_engine = renderer.get_engine();
    CommandList* cmds = render_engine->get_command_list(0, QueueType::Graphics);

    cmds->begin_renderpass(nullptr, nullptr); // TODO: Figure this out
    cmds->bind_pipeline(nullptr);             // And this
    cmds->bind_vertex_buffers({vertex_buffer});
    cmds->bind_index_buffer(index_buffer);
}

nova::renderer::rhi::Pipeline* NuklearDevice::make_pipeline() {
    static std::string vertex_shader;   // TODO
    static std::string fragment_shader; // TODO

    PipelineCreateInfo nk_pipeline_create_info = {};
    nk_pipeline_create_info.name = "NuklearUIPipeline";
    nk_pipeline_create_info.pass = "UI";

    nk_pipeline_create_info.states.push_back(StateEnum::DisableDepthTest);
    nk_pipeline_create_info.states.push_back(StateEnum::DisableCulling);

    nk_pipeline_create_info.states.push_back(StateEnum::Blending);
    nk_pipeline_create_info.source_blend_factor = BlendFactorEnum::SrcAlpha;
    nk_pipeline_create_info.destination_blend_factor = BlendFactorEnum::OneMinusSrcAlpha;

    nk_pipeline_create_info.vertex_fields.emplace_back("Position", VertexFieldEnum::Position);
    nk_pipeline_create_info.vertex_fields.emplace_back("UV0", VertexFieldEnum::UV0);
    nk_pipeline_create_info.vertex_fields.emplace_back("Color", VertexFieldEnum::Color);

    nk_pipeline_create_info.primitive_mode = PrimitiveTopologyEnum::Triangles;

    // TODO: Scissor rectangles, somehow

    // TODO: Nova needs support for ad-hoc pipelines and renderpasses (and command buffers?)

    return nullptr;
}
