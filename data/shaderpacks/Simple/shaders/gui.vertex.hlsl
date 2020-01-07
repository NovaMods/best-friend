struct VsInput {
    float2 position : POSITION;
    float2 uv : TEXCOORD;
    uint color : COLOR;
    float4 data : INDEX;
};

struct VsOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float4 data : INDEX;
};

struct UiConstants {
    float4x4 view_matrix;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UiConstants> constants   : register(b0, space0);

VsOutput main(VsInput input) {
    VsOutput output;

    float4 position = float4(input.position, 0, 1);
    output.position = mul(position, constants.view_matrix);

    output.uv = input.uv;

    uint red = (input.color >> 24) & 0xFF;
    uint green = (input.color >> 16) & 0xFF;
    uint blue = (input.color >> 8) & 0xFF;
    uint alpha = input.color & 0xFF;
    output.color = float4(red, green, blue, alpha) / 255.0f;

    output.data = input.data;

    return output;
}
