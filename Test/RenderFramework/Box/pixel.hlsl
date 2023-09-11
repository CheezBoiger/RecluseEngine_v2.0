

struct PSIn
{
	float4 vPosition 	: SV_POSITION;
	float3 vNormal 		: TEXCOORD0;
	float2 vTexCoord0	: TEXCOORD1;	
	float4 color 		: TEXCOORD2;
};

//Texture2D<float4> g_texture : register(t0);
//SamplerState g_sampler : register(s0);

cbuffer PerVert : register(b0)
{
	float4x4 mModelViewProjection;
	float3x3 mNormal;
	uint   	 useTexturing;
	uint2	 pad0;
};

float4 psMain(PSIn pixIn) : SV_TARGET0
{
	float4 outColor = pixIn.color;
	if (useTexturing == 1)
	{
//		outColor = g_texture.Sample(g_sampler, pixIn.vTexCoord0);
	}
	return outColor;
}