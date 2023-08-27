struct PSInput 
{
	float4 position : SV_POSITION;
};

struct VSInputData 
{
    float2 position : POSITION;
};

struct TestData {
    float4 Color;
    float4 Offset;
};

ConstantBuffer<TestData> Test : register(b0);

PSInput main(VSInputData Input)
{
    PSInput Output;
    Output.position = float4(Input.position.xy + Test.Offset.zw, 0, 1);
    return Output;
}
