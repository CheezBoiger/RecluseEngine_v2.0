//

struct TestData {
    float4 Color;
    float4 Iter;
};


// TODO: Going to have to figure out this case for GLSlang, seems
// that all resources are treated as part of one descriptor set, as well
// as all resources (uav), (srv), (cbv), contain a base binding starting at 0,
// which can potentially overlap with other resources. 
RWTexture2D<float4> ResultImg : register(u0);
ConstantBuffer<TestData> Test : register(b1);

[numthreads(8, 8, 1)]
void main(uint3 UTid : SV_DispatchThreadID)
{
    uint Width = 0;
    uint Height = 0;
    uint Levels = 0;
    
    ResultImg.GetDimensions(0, Width, Height, Levels);
    
    float2 NormCoordinates = (float2(UTid.x, UTid.y) + float2(0.5)) / float2(Width, Height);
    float2 C = (NormCoordinates - float2(0.5, 0.5)) * 2.0 - float2(1.0, 0.0);
    
    float2 Z = float2(0, 0);
    float I = 0;
    for (I = 0.0; I < 1.0; I += Test.Iter.w) {
        Z = float2(
            Z.x * Z.x - Z.y * Z.y + C.x,
            Z.y * Z.x + Z.x * Z.y + C.y
        );
        
        if (length(Z) > 4.0) {
            break;
        }
    }
    
    float4 ToWrite = float4(Test.Color.xyz * I, 1.0);
    ResultImg[UTid.xy] = ToWrite;
}