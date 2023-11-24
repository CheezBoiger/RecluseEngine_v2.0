// Recluse Render Test
struct PSIn
{
	float4 vPosition 	: SV_POSITION;
	float3 vNormal 		: TEXCOORD0;
	float2 vTexCoord0	: TEXCOORD1;	
	float4 color 		: TEXCOORD2;
};

[[vk::binding(0)]] cbuffer PerVert : register(b0)
{
	float4x4 mModelViewProjection;
	float4x4 mNormal;
	uint   	 useTexturing;
	uint3	 pad0;
};

[[vk::binding(1)]] Texture2D<float4> colorTexture : register(t0);
[[vk::binding(2)]] SamplerState colorSampler : register(s0);

struct PSOut 
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 material : SV_TARGET2;
};


PSOut psMain(PSIn pixIn)
{
	PSOut output = { };
	if (useTexturing == 1)
	{
		output.albedo = colorTexture.Sample(colorSampler, pixIn.vTexCoord0);
	}
	else
	{
		output.albedo = pixIn.color;
	}
	output.normal = float4(pixIn.vNormal, 0);
	output.material = float4(0, 0, 0, 0);
	return output;
}