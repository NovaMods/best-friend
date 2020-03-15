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
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct VsOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VsOutput main(VsInput input) {
    VsOutput output;

    Camera camera = camera_matrices.cameras[push_constants.camera_index];
    float4x4 vp = mul(camera.view_matrix, camera.projection_matrix);

    output.position = mul(float4(input.position, 1), vp);
    output.normal = input.normal;

    return output;
}
