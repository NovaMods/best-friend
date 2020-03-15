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
StructuredBuffer<Camera[256]> cameras : register (b0);

/*!
 * \brief Array of all the materials 
 */
[[vk::binding(1, 0)]]
StructuredBuffer<UiMaterial[64]> material_buffer : register (b1);

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

struct VsInput {
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    uint color : COLOR;
    uint texture_id : INDEX;
};

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    uint color : COLOR;
    uint texture_id : INDEX;
};

VsOutput main(VsInput input) {
    VsOutput output;

    float4 position = float4(input.position, 0, 1);
   // output.position = mul(position, constants.view_matrix);

    output.uv = input.uv;

    output.color = input.color;

    output.texture_id = input.texture_id;

    return output;
}
