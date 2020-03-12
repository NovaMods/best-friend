struct Camera {
    float4x4 view;
    float4x4 projection;
};

struct UiMaterial {
    uint texture_idx;
};

/*!
 * \brief All the push constants that are available to a shader that uses the standard pipeline layout
 */
[[vk::push_constant]]
struct StandardPushConstants {
    /*!
     * \brief Index of the material data for the current draw
     */
    uint material_index;
} constants;

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(0, 0)]]
cbuffer cameras : register (b0) {
    Camera camears[256];
};

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(1, 0)]]
cbuffer material_buffer : register (b1) {
    UiMaterial materials[64];
};

/*!
 * \brief Point sampler you can use to sample any texture
 */
[[vk::binding(2, 0)]]
SamplerState point_sampler : register(s0);

/*!
 * \brief Bilinear sampler you can use to sample any texture
 */
[[vk::binding(3, 0)]]
SamplerState bilinear_filter : register(s1);

/*!
 * \brief Trilinear sampler you can use to sample any texture
 */
[[vk::binding(4, 0)]]
SamplerState trilinear_filter : register(s3);

/*!
 * \brief Array of all the textures that are available for a shader to sample from
 */
[[vk::binding(5, 0)]]
Texture2D textures[] : register(t0);

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
    
    float4 vertex_color = float4(red, green, blue, alpha) / 255.0f;

    float4 texture_color = textures[input.texture_id].Sample(point_sampler, input.uv);
    return vertex_color * texture_color;
}
