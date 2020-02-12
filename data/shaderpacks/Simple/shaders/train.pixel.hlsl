struct VsOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

float4 main(VsOutput input) : SV_Target {
    return float4(input.normal, 1);
}
