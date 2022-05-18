Texture2D<float4> srcText : register(t0);

SamplerState samState : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return srcText.Sample(samState, input.Tex);
}
