// Recluse Render Test
struct PSIn
{
	float4 vPosition : SV_POSITION;
	float2 vTexCoord : TEXCOORD0;
};

#define LIGHT_TYPE_UNKNOWN 			0
#define LIGHT_TYPE_POINT 			1
#define LIGHT_TYPE_DIRECTIONAL 		2
#define LIGHT_TYPE_SPOT 			3

struct Light
{
	int 	LightType;
	float 	Position;
	float 	Direction;
	float 	Attenuation;
	
	float4 Color;
};


[[vk::binding(0)]] cbuffer SceneCamera : register(b0)
{
	float4x4 ViewProjection;
	float4x4 View;
};

[[vk::binding(1)]] cbuffer LightView : register(b1)
{
	uint numLights;
	float3 pad0;
};

[[vk::binding(2)]] Texture2D<float4> AlbedoTexture 	: register(t0);
[[vk::binding(3)]] Texture2D<float4> NormalTexture 	: register(t1);
[[vk::binding(4)]] Texture2D<float4> MaterialTexture 	: register(t2);
[[vk::binding(5)]] Texture2D<float4> DepthTexture : register(t3);

[[vk::binding(6)]] StructuredBuffer<Light> LightBuffer : register(t4);

[[vk::binding(7)]] SamplerState g_sampler : register(s0);

float4 psMain(PSIn pixIn) : SV_TARGET0
{
	float4 color = float4(0, 0, 0, 0);
	float2 UV = pixIn.vTexCoord;
	color = AlbedoTexture.Sample(g_sampler, UV).rgba;
	for (uint lightIdx = 0; lightIdx < numLights; ++lightIdx)
	{
		Light light = LightBuffer[lightIdx];
		color += light.Color;
	}
	return color;
}