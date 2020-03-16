struct MaterialData {
    uint texture_idx;
};

#include "nova/standard_pipeline_layout.hlsl"

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

    Camera camera = cameras.Load(constants.camera_index);

    float4 position = float4(input.position, 0, 1);
    output.position = mul(position, camera.view);

    output.uv = input.uv;

    output.color = input.color;

    output.texture_id = input.texture_id;

    return output;
}
