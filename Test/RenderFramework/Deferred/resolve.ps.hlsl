// Recluse Render Test
struct PSIn
{
	float4 vPosition : SV_POSITION;
	float2 vTexCoord : TEXCOORD0;
};


[[vk::binding(0)]] cbuffer SceneCamera : register(b0)
{
	float4x4 ViewProjection;
	float4x4 View;
};

[[vk::binding(1)]] cbuffer LightView : register(b1)
{
	float4 	Position;
	float 	Radius;
	float 	Strength;
	float2 	Pad0;
};

[[vk::binding(2)]] Texture2D<float4> AlbedoTexture 	: register(t0);
[[vk::binding(3)]] Texture2D<float4> NormalTexture 	: register(t1);
[[vk::binding(4)]] Texture2D<float4> MaterialTexture 	: register(t2);
[[vk::binding(5)]] Texture2D<float4> DepthTexture : register(t3);

[[vk::binding(6)]] SamplerState g_sampler : register(s0);

float4 psMain(PSIn pixIn) : SV_TARGET0
{
	float4 color = float4(0, 0, 0, 0);
	float2 UV = pixIn.vTexCoord;
	color = AlbedoTexture.Sample(g_sampler, UV).rgba;
	return color;
}