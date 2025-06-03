struct MeshletOutput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};


[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void msmain(out indices uint3 triangles[1], out vertices MeshletOutput vertices[3])
{
	SetMeshOutputCounts(3, 1);
	triangles[0] = uint3(0, 1, 2);
	vertices[0].position = float4(-0.5,  0.5, 0.0, 1.0);
	vertices[1].position = float4( 0.5,  0.5, 0.0, 1.0);
	vertices[2].position = float4( 0.0, -0.5, 0.0, 1.0);
	
	vertices[0].color = float3(1.0, 0.0, 0.0);
	vertices[1].color = float3(0.0, 1.0, 0.0);
	vertices[2].color = float3(0.0, 0.0, 1.0);
}


float4 psmain(MeshletOutput psIn) : SV_TARGET0
{
	return float4(psIn.color, 1);
}
