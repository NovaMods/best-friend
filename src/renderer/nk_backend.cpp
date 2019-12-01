#include "nk_backend.hpp"

#include <nova_renderer/nova_renderer.hpp>

#define NK_IMPLEMENTATION
#include <nuklear.h>

using namespace nova::renderer;
using namespace shaderpack;

nk_context* nk_nova_init(NovaRenderer& renderer) {
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
}
