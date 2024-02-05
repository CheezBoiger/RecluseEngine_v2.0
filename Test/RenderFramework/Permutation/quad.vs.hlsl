//
struct VSOut
{
	float4 vPosition : SV_POSITION;
	float2 vTexCoord : TEXCOORD0;
};


VSOut Main(uint VertexIndex : SV_VERTEXID)
{
	VSOut output;
	output.vTexCoord = float2((VertexIndex << 1) & 2, VertexIndex & 2);
	output.vPosition = float4(output.vTexCoord * 2.0 - 1.0, 0.0, 1.0);
	return output;
}