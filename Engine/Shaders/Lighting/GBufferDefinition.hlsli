//
#ifndef GBUFFER_DEFINITION_HLSLI
#define GBUFFER_DEFINITION_HLSLI

struct GBuffer {
	float3 albedo;
	float  roughness;
	float  metallic;
	float  gloss;
	float  ao;
	float3 emission;
};


void unpackGBuffer(inout GBuffer data, in float4 rt0, in float4 rt1, in float4 rt2, in float4 rt3)
{
}


void packGBuffer(in GBuffer data, in float4 rt0, in float4 rt1, in float4 rt2, in float4 rt3)
{
}

#endif // GBUFFER_DEFINITION_HLSLI