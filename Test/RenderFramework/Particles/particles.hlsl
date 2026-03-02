// Particle compute and render test.

//////////////////////////////////////////////
// Particle Common
//////////////////////////////////////////////

struct Particle
{
	float3 	position;
	float 	lifeSpan;
	
	float3 	velocity;
	float 	pad0;
};


struct ParticleConstants
{
	float deltaTime;
	float mass;
	float volume;
	uint maxParticles;
	
	float3 gravity;
	float lifeSpan;
};


ConstantBuffer<ParticleConstants> ParticleGlobal 	: register(b0);

//////////////////////////////////////////////
// Particle Simple Compute
//////////////////////////////////////////////

RWStructuredBuffer<Particle> ParticleBuffer 		: register(t0);

//////////////////////////////////////////////
// Particle Fluid Compute
//////////////////////////////////////////////

RWByteAddressBuffer ParticlePositionBuffer 	 		: register(t0);
RWByteAddressBuffer ParticleVelocityBuffer     		: register(t1);
ByteAddressBuffer DeadParticleBuffer 		 		: register(t2);

RWByteAddressBuffer Grid 				 			: register(t3);
RWByteAddressBuffer ParticleGridIndices 			: register(t4);
RWByteAddressBuffer ParticleGridSortedIndices 		: register(t5);

//////////////////////////////////////////////
// Particle Render
//////////////////////////////////////////////
struct VsIn
{
	float3 localPosition 	: POSITION;
	float3 localNormal 		: NORMAL0;
	float2 uv0 				: TEXCOORD0;
	float2 uv1 				: TEXCOORD1;
};

struct PsIn
{
	float4 position 		: SV_POSITION;
	float4 normal 			: NORMAL0;
	float2 uv0 				: TEXCOORD0;
	float2 uv1 				: TEXCOORD1;
};


struct PsOut
{
	float4 rt0 : SV_TARGET0;
};


PsIn ParticleVertexMain(VsIn vsIn, uint instanceId : SV_InstanceID)
{
	PsIn psIn;
	return psIn;
}


PsOut ParticlePixelMain(PsIn psIn)
{
	PsOut psOut;
	
	return psOut;
}

//////////////////////////////////////////////

[numthreads(64, 1, 1)]
void ParticleComputeFluidMain(uint3 dtid : SV_DispatchThreadID)
{
	const uint threadIdx = dtid.x;
	
	// Compute particle simulation will be using FLIP and PIC for grid simulation.
	// Move particle to grid for simulation
	// Update particle velocities from the grid
	// Move grid info back to particles
	// Perform Advection
}


[numthreads(64, 1, 1)]
void ParticleComputeSimpleMain(uint3 dtid : SV_DispatchThreadID)
{
	const uint threadIdx = dtid.x;
	if (threadIdx >= ParticleGlobal.maxParticles) return;
	
	Particle particle = ParticleBuffer[threadIdx];
	
	if (particle.lifeSpan > 0.0f)
	{
		particle.velocity += ParticleGlobal.gravity * deltaTime;
		particle.position += particle.velocity * deltaTime;
		particle.lifeSpan -= deltaTime;
	}
	else
	{
		// Reset the particle.
	}
	
	// Store the resolve back to the buffer.
	ParticleBuffer[threadIdx] = particle;
}