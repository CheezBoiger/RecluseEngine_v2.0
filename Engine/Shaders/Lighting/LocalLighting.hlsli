//
#ifndef LOCAL_LIGHTING_HLSLI
#define LOCAL_LIGHTING_HLSLI

struct HemisphericAmbientConstants {
	float3 ambientDown;
	float3 ambientRange;
};

// Calculate Hemispheric ambient light.
float3 computeHemisphericAmbient(float3 normal, float3 albedo, in HemisphericAmbientConstants hemi)
{	
	float up = normal.y * 0.5 + 0.5;
	float3 ambient = hemi.ambientDown + up * hemi.ambientRange;
	return ambient * albedo;
}

#endif // LOCAL_LIGHTING_HLSLI