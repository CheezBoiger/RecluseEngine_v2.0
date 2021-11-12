//
#ifndef AMBIENT_OCCLUSION_HLSLI
#define AMBIENT_OCCLUSION_HLSLI

// Implementation of Jorge Jimenez, "Practical Realtime Strategies for Accurate Indirect Occlusion"

float indirectIlluminationAndAO(float visibility, float3 albedo)
{
	float3 a =  2.0404 * albedo - 0.3324;
	float3 b = -4.7951 * albedo + 0.6417;
	float3 c =  2.7552 * albedo + 0.6903;
	
	float occlusion = ((a * visibility + b) * visibility + c) * visibility;
	
	return max(visibility, occlusion);
}

#endif // AMBIENT_OCCLUSION_HLSLI