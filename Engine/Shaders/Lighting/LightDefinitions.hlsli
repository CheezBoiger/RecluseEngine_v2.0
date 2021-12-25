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
	uint 	type;
	
	float3 	position;
	float3 	direction;
	
	// Shadow index.
	uint   	shadowIndex;	// shadow index from direction of light.
	uint 	goboIndex;		// Gobo texture index.
	
	// Angle info
	float 	angleOuter;
	float	angleInner;
	
	float	radius;			// distance - radius max before light is no longer contributing.
};


struct Shadow {
	float 		bias;
	float 		occlusionScale;
	uint 		textureIndex;		// texture index that this shadow pertains to.
	float		pad0; 				// reserved.
	float4x4 	lightMatrix;	
};

#endif // LIGHT_DEFINITIONS_HLSLI