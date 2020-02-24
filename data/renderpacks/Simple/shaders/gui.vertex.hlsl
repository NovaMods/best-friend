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

struct UiConstants {
    float4x4 view_matrix;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UiConstants> constants : register(b0, space0);

VsOutput main(VsInput input) {
    VsOutput output;

    float4 position = float4(input.position, 0, 1);
    output.position = mul(position, constants.view_matrix);

    output.uv = input.uv;

    output.color = input.color;

    output.texture_id = input.texture_id;

    return output;
}
