[[vk::binding(0, 1)]]
Texture2D ui_textures[] : register(t0);

[[vk::binding(1, 1)]]
SamplerState ui_sampler : register(s0);

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    uint texture_id : INDEX;
};

float4 main(VsOutput input) : SV_Target {
    float4 texture_color = ui_textures[input.texture_id].Sample(ui_sampler, input.uv);
    return input.color * texture_color;
}
