struct VsInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VsOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VsOutput main(VsInput input) {
    VsOutput output;

    output.position = float4(input.position, 1);
    output.normal = input.normal;

    return output;
}
