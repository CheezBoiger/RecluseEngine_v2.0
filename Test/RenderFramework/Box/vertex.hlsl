// Recluse Render Test

struct VSOut
{
	float4 vPosition 	: SV_POSITION;
	float3 vNormal 		: TEXCOORD0;
	float2 vTexCoord0	: TEXCOORD1;	
	float4 color 		: TEXCOORD2;
};


[[vk::binding(5)]] cbuffer PerVert : register(b0)
{
	float4x4 mModelViewProjection;
	float4x4 mNormal;
	uint   	 useTexturing;
	uint3	 pad0;
};


struct VSIn
{
	float3 vPosition : POSITION; 
	float3 vNormal		: TEXCOORD0; 
	float2 vTexCoord : TEXCOORD1; 
	float4 color		: TEXCOORD2;
};


VSOut Main(VSIn inVerts)
{
	VSOut Out;
	float4 localPos = float4(inVerts.vPosition.xyz, 1);
	Out.vPosition = mul(mModelViewProjection, localPos);
	Out.vNormal = mul(float4(inVerts.vNormal.xyz, 1.0), mNormal);
	Out.color = inVerts.color;
	Out.vTexCoord0 = inVerts.vTexCoord;
	return Out;
}