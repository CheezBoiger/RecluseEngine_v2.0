// Recluse Render Test

struct VSOut
{
	float3 vPosition 	: SV_POSITION;
	float3 vNormal 		: TEXCOORD0;
	float2 vTextCoord0	: TEXCOORD1;	
	float4 color 		: TEXCOORD2;
};


cbuffer PerVert : register(b0)
{
	float4x4 mModelViewProjection;
	float3x3 mNormal;
	uint   	 useTexturing;
	uint2	 pad0;
};


VSOut Main(float3 vPosition, float3 vNormal, float2 vTexCoord)
{
	VSOut Out = { };
	Out.vPosition = mul(mPosition, mModelViewProjection);
	Out.vNormal = mul(vNormal, mNormal);
	Out.color = color;
	Out.vTexCoord0 = vTextCoord;
	return Out;
}