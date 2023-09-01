//
struct TestData {
    float4 Color;
    float4 Offset;
};

struct PSInput {
    float4 Position : SV_POSITION;
};

ConstantBuffer<TestData> Test : register(b0);

float4 main(PSInput Input) : SV_TARGET0
{
    return Test.Color;
}