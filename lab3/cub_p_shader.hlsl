#include "common_shader.fx"

Texture2D<float4> SphText : register (t0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 pos = normalize(input.Norm);
    float u = 1.0f - atan2(pos.z, pos.x) / (2 * M_PI);
    float v = 0.5f - asin(pos.y) / M_PI;
    return float4(SphText.Sample(samLinear, float2(u, v)).xyz, 1.0f);
}