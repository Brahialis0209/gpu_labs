#define NUM_OF_LIGHT 3
cbuffer ConstantBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float4 vLightDir[NUM_OF_LIGHT];
	float4 vColor[NUM_OF_LIGHT];
	float4 vIntensity[NUM_OF_LIGHT];
	float4 Eye;
	float metalness;
	float roughness;
	float3 albedo;
};

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
	float4 WorldPos : TEXCOORD2;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return txDiffuse.Sample(samLinear, input.TexCoord);
}