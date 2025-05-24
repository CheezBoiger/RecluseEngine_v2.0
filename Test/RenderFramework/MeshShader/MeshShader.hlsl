// Mesh Shader impl

struct MeshletOutput
{
	float position : SV_POSITION;
	float3 color : COLOR;
};


[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msmain(out indices uint3 triangles[1], out vertices MeshletOutput vertices[3])
{
	SetMeshCounts(3, 1);
	triangles[0] = uint(0, 1, 2);
	vertices[0].position = float4(-0.5,  0.5, 0.0, 1.0);
	vertices[1].position = float4( 0.5,  0.5, 0.0, 1.0);
	vertices[2].position = float4( 0.0, -0.5, 0.0, 1.0);
	
	vertices[0].color = float3(1.0, 0.0, 0.0);
	vertices[1].color = float3(0.0, 1.0, 0.0);
	vertices[2].color = float3(0.0, 0.0, 1.0);
}


void psmain(MeshletOutput psIn) : SV_TARGET0
{
	return psIn.color;
}
