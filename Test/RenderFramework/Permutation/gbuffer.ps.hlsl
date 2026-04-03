// Recluse Render Test

struct PSIn
{
	float4 vPosition 	: SV_POSITION;
	float3 vNormal 		: TEXCOORD0;
	float2 vTexCoord0	: TEXCOORD1;	
	float4 color 		: TEXCOORD2;
};

cbuffer PerVert : register(b0)
{
	float4x4 mModelViewProjection;
	float4x4 mNormal;
	uint   	 useTexturing;
	uint 	 pad0[3];
};

cbuffer Scene : register(b1)
{
	float	 time;
	float    deltaTime;
	uint 	 moveLeft;
	uint     moveUp;
};

Texture2D<float4> colorTexture 	: register(t0);
SamplerState colorSampler 		: register(s0);

struct PSOut 
{
	float4 albedo : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 material : SV_TARGET2;
};


PSOut psMain(PSIn pixIn)
{
	PSOut Output;
#if USE_TEXTURE
	float2 texcoord = pixIn.vTexCoord0;
#if (MOVING_TEXTURE_X == 1)
	if (moveLeft == 1)
		texcoord.x += time;
#endif
#if (MOVING_TEXTURE_Y == 1)
	if (moveUp == 1)
		texcoord.y += time;
#endif
	texcoord = select(texcoord > 1.0, frac(texcoord), texcoord);
	
	Output.albedo = colorTexture.Sample(colorSampler, texcoord);
#else
		Output.albedo = pixIn.color;
#endif
	Output.normal = float4(pixIn.vNormal, 0);
	Output.material = float4(0, 0, 0, 0);
	return Output;
}