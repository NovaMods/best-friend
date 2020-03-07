struct PushConstants {
    uint camera_index;
    uint material_index;
    uint material_instance_index;
};

struct VsInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct VsOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

[[vk::push_constant]]
PushConstants push_constants;

struct Camera {
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct CameraArray {
    Camera cameras[256];
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraArray> camera_matrices : register(b0, space0);

VsOutput main(VsInput input) {
    VsOutput output;

    Camera camera = camera_matrices.cameras[push_constants.camera_index];
    float4x4 mvp = mul(camera.projection_matrix, camera.view_matrix);

    output.position = mul(float4(input.position, 1), mvp);
    output.normal = input.normal;

    return output;
}
