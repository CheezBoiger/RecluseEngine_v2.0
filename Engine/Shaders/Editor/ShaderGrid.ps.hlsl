// Adapted from http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
// 

cbuffer Camera : register(b0)
{
	float4x4 WorldToView;	// View 
	float4x4 ViewToWorld; 	// inverse
	float4x4 ViewToClip;	// Projection
	float4x4 ClipToView;	// inverse
	float4x4 WorldToClip	// View * Projection
	float3   WorldPosition;
	float	 Pad0;
	
	float    Near;
	float 	 Far;
	float2   Pad1;
};

float4 Grid(float3 PixelPos3D, float Scale, uint drawAxis)
{
	float2 Coord 		= PixelPos3D.xz * Scale;
	float2 Derivative 	= fwidth(Coord);
	float2 Gridd 		= abs(fract(Coord - 0.5) - 0.5) / Derivative;
	float Line 			= min(Gridd.x, Gridd.y);
	float MinZ 			= min(Derivative.y, 1);
	float MinX 			= min(Derivative.x, 1);
	
	float4 Color 		= float4(0.2, 0.2, 0.2, 1.0 - min(Line, 1.0));
	
	if (PixelPos3D.x > -0.1 * MinX && PixelPos3D.x < 0.1 * MinX)
	{
		Color.z = 1.0;
	}
	
	if (PixelPos3D.z > -0.1 * MinZ && PixelPos3D.z < 0.1 * MinZ)
	{
		Color.x = 1.0;
	}
	
	return Color;
}


float ComputeDepth(float3 Pos)
{
	float4 ClipPos = mul(WorldToClip, float4(Pos.xyz, 1.0));
	return ClipPos.z / ClipPos.w;
}

float LinearizeDepth(float3 Pos)
{
	float4 ClipP 		= mul(WorldToClip, float4(Pos, 1.0));
	float ClipDepth 	= (ClipP.z / ClipP.w) * 2.0 - 1.0;
	float LinearDepth 	= (2.0 * Near * Far) / (Far + Near - ClipDepth * (Far - Near));
	return LinearDepth / Far;
}

struct PixelIn
{
	float4 PosClip   : SV_POSITION;
	float3 NearPoint : TEXCOORD0;
	float3 FarPoint  : TEXCOORD1;
};


struct PixelOut 
{
	float4 Color 	: SV_Target0;
	float Depth 	: SV_Depth;
};


PixelOut MainPs(PixelIn Pixel)
{
	PixelOut Result 	= { };
	float T 			= -Pixel.NearPoint.y / (Pixel.FarPoint.y - Pixel.NearPoint.y);
	float3 PixelPos3D 	= Pixel.NearPoint + t * (Pixel.FarPoint - Pixel.NearPoint);
	
	Result.Depth 		= ComputeDepth(PixelPos3D);
	
	float LinearDepth 	= LinearizeDepth(PixelPos3D);
	float Fading 		= max(0, (0.5 - LinearDepth));
	
	Result.Color 		= (Grid(PixelPos3D, 10, 1) + Grid(PixelPos3D, 1, 1)) * float(t > 0);
	Result.Color.a 		*= Fading;
	
	return Result;
}