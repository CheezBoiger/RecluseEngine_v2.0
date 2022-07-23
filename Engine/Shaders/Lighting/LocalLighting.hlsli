//
#ifndef LOCAL_LIGHTING_HLSLI
#define LOCAL_LIGHTING_HLSLI

struct HemisphericAmbientConstants {
	float3 AmbientDown;
	float3 AmbientRange;
};

// Calculate Hemispheric ambient light.
float3 ComputeHemisphericAmbient(float3 Normal, float3 Albedo, in HemisphericAmbientConstants Hemi)
{	
	float up = Normal.y * 0.5 + 0.5;
	float3 ambient = Hemi.AmbientDown + up * Hemi.AmbientRange;
	return ambient * Albedo;
}

#endif // LOCAL_LIGHTING_HLSLI