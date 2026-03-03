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
	
	float3 particleResetPosition;
	float pad0;
	float3 particleResetVelocity;
	float pad1;
};


ConstantBuffer<ParticleConstants> ParticleGlobal 	: register(b0);

//////////////////////////////////////////////
// Particle Simple Compute
//////////////////////////////////////////////

RWStructuredBuffer<Particle> ParticleBuffer 		: register(u0);

//////////////////////////////////////////////
// Particle Fluid Compute
//////////////////////////////////////////////

RWByteAddressBuffer ParticlePositionBuffer 	 		: register(u0);
RWByteAddressBuffer ParticleVelocityBuffer     		: register(u1);
ByteAddressBuffer DeadParticleBuffer 		 		: register(t2);

RWByteAddressBuffer Grid 				 			: register(u2);
RWByteAddressBuffer ParticleGridIndices 			: register(u3);
RWByteAddressBuffer ParticleGridSortedIndices 		: register(u4);

//////////////////////////////////////////////
// Particle Render
//////////////////////////////////////////////

cbuffer SceneConstants : register(b1)
{
	float4x4 ViewProjection;
	float4x4 InverseViewProjection;
	
	float3 CameraWorldPosition;
	float deltaTime;
};

StructuredBuffer<Particle> ReadParticleBuffer 		: register(t0);

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
	Particle particle = ReadParticleBuffer[instanceId];
	psIn.position = mul(particle.position, ViewProjection);
	return psIn;
}


PsOut ParticlePixelMain(PsIn psIn)
{
	PsOut psOut;
	psOut.rt0 = float4(1, 0, 0, 1);
	return psOut;
}

//////////////////////////////////////////////

// Fluid simulation Needs to be split.
[numthreads(64, 1, 1)]
void FluidParticleToGrid(uint3 dtid : SV_DispatchThreadID)
{
	const uint threadIdx = dtid.x;
	
	// Compute particle simulation will be using FLIP and PIC for grid simulation.
	// Move particle to grid for simulation
	// Update particle velocities from the grid
	// Move grid info back to particles
	// Perform Advection
}

[numthreads(64, 1, 1)]
void FluidParticleFLIP(uint3 dtid : SV_DispatchThreadID)
{
}

[numthreads(64, 1, 1)]
void FluidParticleAdvection(uint3 dtid : SV_DispatchThreadID)
{
}


// Using a simple particle simulator.
// Nothing fancy.
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
		particle.lifeSpan = ParticleGlobal.lifeSpan;
		particle.position = ParticleGlobal.particleResetPosition;
		particle.velocity = ParticleGlobal.particleResetVelocity;
	}
	
	// Store the resolve back to the buffer.
	ParticleBuffer[threadIdx] = particle;
}