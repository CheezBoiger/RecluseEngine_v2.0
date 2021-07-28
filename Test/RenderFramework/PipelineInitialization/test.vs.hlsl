//
struct PSInput {
    float4 Position : SV_POSITION;
};

struct VSInputData {
    float2 Position : POSITION0;
};

struct TestData {
    float4 Color;
    float4 Offset;
};

ConstantBuffer<TestData> Test : register(b0);

PSInput main(VSInputData Input)
{
    PSInput Output;
    Output.Position = float4(Input.Position + Test.Offset.xy, 0, 1);
    return Output;
}
