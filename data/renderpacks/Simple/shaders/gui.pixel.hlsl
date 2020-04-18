struct MaterialData {
    uint texture_idx;
};

#include "nova/standard_pipeline_layout.hlsl"

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    uint color : COLOR;
    uint texture_id : INDEX;
};

float4 main(VsOutput input) : SV_Target {
    uint red = input.color & 0xFF;
    uint green = (input.color >> 8) & 0xFF;
    uint blue = (input.color >> 16) & 0xFF;
    uint alpha = (input.color >> 24) & 0xFF;

    MaterialData material = material_buffer.Load(constants.material_index);
    
    float4 vertex_color = float4(red, green, blue, alpha) / 255.0f;

    float4 texture_color = textures[NonUniformResourceIndex(material.texture_idx)].Sample(point_sampler, input.uv);
    return vertex_color * texture_color;
}
