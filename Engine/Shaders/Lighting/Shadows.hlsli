//
#ifndef SHADOWS_HLSLI
#define SHADOWS_HLSLI


// Obtain the blend amount for our cascaded shadow maps.
void CalculateBlendAmountForMap
	(
		in float4 ShadowMapTextureCoord, 
		in float CascadeBlendArea, 
		in out float CurrentPixelsBlendBandLocation, 
		out float BlendBetweenCascadesAmount
	)
{
	float2 distanceToOne 					= float2(1 - ShadowMapTextureCoord.x, 1 - ShadowMapTextureCoord.y);
	CurrentPixelsBlendBandLocation 			= min(ShadowMapTextureCoord.x, ShadowMapTextureCoord.y);
	float currentPixelsBlendBandLocation2 	= min(distanceToOne.x, distanceToOne.y);
	CurrentPixelsBlendBandLocation 			= min(CurrentPixelsBlendBandLocation, currentPixelsBlendBandLocation2);
	BlendBetweenCascadesAmount 				= CurrentPixelsBlendBandLocation / CascadeBlendArea;
}

#endif // SHADOWS_HLSLI