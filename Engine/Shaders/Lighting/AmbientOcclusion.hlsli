//
#ifndef AMBIENT_OCCLUSION_HLSLI
#define AMBIENT_OCCLUSION_HLSLI

// Implementation of Jorge Jimenez, "Practical Realtime Strategies for Accurate Indirect Occlusion"

float IndirectIlluminationAndAO(float Visibility, float3 Albedo)
{
	float3 a =  2.0404 * Albedo - 0.3324;
	float3 b = -4.7951 * Albedo + 0.6417;
	float3 c =  2.7552 * Albedo + 0.6903;
	
	float occlusion = ((a * Visibility + b) * Visibility + c) * Visibility;
	
	return max(Visibility, occlusion);
}


// Simple Implementation of Specular Occlusion from Sabastien Lagarde.
float ComputeSpecOcclusion(float NoV, float Ao, float Roughness)
{
	return saturate(pow(NoV + Ao, exp2(-16 * Roughness - 1)) - 1 + Ao);
}
#endif // AMBIENT_OCCLUSION_HLSLI