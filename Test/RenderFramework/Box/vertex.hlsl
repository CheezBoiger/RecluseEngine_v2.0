// Recluse Render Test

struct VSOut
{
	float3 vPosition 	: SV_POSITION;
	float3 vNormal 		: TEXCOORD0;
	float4 color 		: TEXCOORD1;
};


cbuffer PerVert : register(c0)
{
	float4x4 mModelViewProjection;
	float3x3 mNormal;
	float    pad0;
	
	float4   color;
};


VSOut Main(float3 vPosition, float3 vNormal)
{
	VSOut Out = { };
	Out.vPosition = mul(mPosition, mModelViewProjection);
	Out.vNormal = mul(vNormal, mNormal);
	Out.color = color; 
	return Out;
}