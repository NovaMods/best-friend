[[vk::binding(0, 1)]]
SamplerState ui_sampler : register(s0);

[[vk::binding(1, 1)]]
Texture2D ui_textures[256] : register(t0);

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    uint texture_id : INDEX;
};

float4 main(VsOutput input) : SV_Target {
    uint red = (input.color >> 24) & 0xFF;
    uint green = (input.color >> 16) & 0xFF;
    uint blue = (input.color >> 8) & 0xFF;
    uint alpha = input.color & 0xFF;
    float4 vertex_color = float4(red, green, blue, alpha) / 255.0f;

    // float4 texture_color = ui_textures[input.texture_id].Sample(ui_sampler, input.uv);
    return vertex_color;// * texture_color;
}
