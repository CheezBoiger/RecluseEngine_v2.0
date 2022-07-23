//
#ifndef GBUFFER_DEFINITION_HLSLI
#define GBUFFER_DEFINITION_HLSLI

struct GBuffer {
	float3 Albedo;
	float  Roughness;
	float  Metallic;
	float  Gloss;
	float  Ao;
	float3 Emission;
};


void UnpackGBuffer(inout GBuffer Data, in float4 Rt0, in float4 Rt1, in float4 Rt2, in float4 Rt3)
{
}


void PackGBuffer(in GBuffer Data, in float4 Rt0, in float4 Rt1, in float4 Rt2, in float4 Rt3)
{
}

#endif // GBUFFER_DEFINITION_HLSLI