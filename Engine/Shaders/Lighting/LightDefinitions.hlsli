//
#ifndef LIGHT_DEFINITIONS_HLSLI
#define LIGHT_DEFINITIONS_HLSLI

#define LIGHT_TYPE_SPOT     (1)
#define LIGHT_TYPE_POINT	(2)
#define LIGHT_TYPE_DIRECT	(3)
#define LIGHT_TYPE_AREA 	(4)

#define MAX_LIGHTS_SPOT		(512) 
#define MAX_LIGHTS_POINT	(512)
#define MAX_LIGHTS_DIRECT	(512)
#define MAX_LIGHTS_AREA		(512)

#define MAX_LIGHTS_IBL		(512)

struct Light {
	uint 	Type;
	
	float3 	Position;
	float3 	Direction;
	
	// Shadow index.
	uint   	ShadowIndex;	// shadow index from direction of light.
	uint 	GoboIndex;		// Gobo texture index.
	
	// Angle info
	float 	AngleOuter;
	float	AngleInner;
	
	float	radius;			// distance - radius max before light is no longer contributing.
};


struct Shadow {
	float 		Bias;
	float 		OcclusionScale;
	uint 		TextureIndex;		// texture index that this shadow pertains to.
	float		Pad0; 				// reserved.
	float4x4 	LightMatrix;	
};

#endif // LIGHT_DEFINITIONS_HLSLI