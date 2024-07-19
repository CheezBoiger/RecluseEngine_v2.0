// Adapted from http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
// 

cbuffer Camera : register(b0)
{
	float4x4 WorldToView;	// View
	float4x4 ViewToWorld; 	// inverse
	float4x4 ViewToClip;	// Projection
	float4x4 ClipToView;	// inverse
	float4x4 WorldToClip;	// View * Projection
	float3   WorldPosition;
	float	 Pad0;
	
	float    Near;
	float 	 Far;
	float2   Pad1;
};

struct PixelIn 
{
	float4 PosClip   : SV_POSITION;
	float3 NearPoint : TEXCOORD0;
	float3 FarPoint  : TEXCOORD1;
};


static float3 GridPlane[6] = { 
	float3( 1,  1, 0), float3(-1, -1, 0), float3(-1,  1, 0),
	float3(-1, -1, 0), float3( 1,  1, 0), float3( 1, -1, 0)
};

float3 UnprojectPoint(float x, float y, float z)
{
	// Unproject our point from clip to world.
	float4 UnprojectedPoint = mul(ClipToView, float4(x, y, z, 1.0));
	UnprojectedPoint 			= mul(ViewToWorld, UnprojectedPoint);
	// Perspective divide.
	return UnprojectedPoint.xyz / UnprojectedPoint.w;
}

PixelIn MainVs(uint VertId : SV_VertexID)
{
	PixelIn Pixel;
	
	float3 ClipP 	= GridPlane[VertId].xyz;
	Pixel.PosClip 	= float4(ClipP.xyz, 1.0);
	Pixel.NearPoint = UnprojectPoint(ClipP.x, ClipP.y, 0.0);
	Pixel.FarPoint 	= UnprojectPoint(ClipP.x, ClipP.y, 1.0);
	
	return Pixel;
}