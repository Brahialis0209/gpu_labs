#include "common_shader.fx"

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n = input.Norm.xyz;
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 color = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < NUM_OF_LIGHT; i++)
    {
        float3 l = normalize(vLightDir[i].xyz - input.WorldPos.xyz);
        float3 LO_i = BRDF(n, v, l) * vColor[i].xyz * vIntensity[i].x * max(dot(l, n), 0);
        color += LO_i;
    }
    color += AmbientIRradiance(n, v).xyz;
    return float4(color, 1.0f);
}