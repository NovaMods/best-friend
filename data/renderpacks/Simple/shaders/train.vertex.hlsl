struct MaterialData {
    uint texture_idx;
};

#include "nova/standard_pipeline_layout.hlsl"

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

    Camera camera = cameras.Load(constants.camera_index);
    float4x4 vp = mul(camera.view, camera.projection);

    output.position = mul(float4(input.position, 1), vp);
    output.normal = input.normal;

    return output;
}
