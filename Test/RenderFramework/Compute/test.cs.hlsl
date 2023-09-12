// THIS IS NOT MY CODE
// This is an HLSL implementation of Arne Stenkrona's Mandelbrot.
// I am merely using it for testing!
// Please find below, the excellent example of their implementation.
// https://arnestenkrona.github.io/blog/2021/03/04/Mandelbrot-in-Shadertoy

struct TestData {
    float4 Color;
    float4 Iter;
	float4 Resolution;
	float  Time;
	float3 Pad0;
};


// TODO: Going to have to figure out this case for GLSlang, seems
// that all resources are treated as part of one descriptor set, as well
// as all resources (uav), (srv), (cbv), contain a base binding starting at 0,
// which can potentially overlap with other resources. Currently, using this decorator will 
// suffice.
[[vk::binding(1)]] RWTexture2D<float4> ResultImg : register(u0);
[[vk::binding(0)]] ConstantBuffer<TestData> Test : register(b1);

static const float3 Palette[8] = {  float3( 0.0, 0.0, 0.0 ),
                                float3( 0.5, 0.5, 0.5 ),
                                float3( 1.0, 0.5, 0.5 ),
                                float3( 0.5, 1.0, 0.5 ),
                                float3( 0.5, 0.5, 1.0 ),
                                float3( 0.5, 1.0, 1.0 ),
                                float3( 1.0, 0.5, 1.0 ),
                                float3( 1.0, 1.0, 0.5 ) };


float3 Mandelbrot(float2 Coord)
{
	float3 Color;
	float Depth = 16.0;
	float Scale = 1.5 / pow(2.0, Depth * abs(sin(Test.Time / Depth)));
	float2 Offset = float2(-1.5, 0.0);
	
	float AspectRatio = Test.Resolution.x / Test.Resolution.y;
	float2 UV = Coord.xy / Test.Resolution.xy;
	float Re0 = Scale * 2.0 * (2.0 * (Coord.x / Test.Resolution.x) - 1.0) + Offset.x;
	float Im0 = Scale * 2.0 * (1.0 / AspectRatio) * (2.0 * (Coord.y / Test.Resolution.y) - 1.0) + Offset.y;
	
	bool Diverged = false;
	float Re = Re0;
	float Im = Im0;
	int i;
	for (i = 0; i < 200; ++i)
	{
		if (Re * Re + Im * Im > 2000.0 * 2000.0)
		{
			Diverged = true;
			break;
		}
		float Retemp = Re * Re - Im * Im + Re0;
		Im = 2.0 * Re * Im + Im0;
		Re = Retemp;
	}
	
	if (Diverged)
	{
		int NumPalette = 8;
		float GradScale = 1.0;
		float Smoothed = log2(log2(Re * Re + Im * Im) / 2.0);
		float ColorIndex = (sqrt(float(i) + 10.0 - Smoothed) * GradScale);
		
		float ColorLerp = frac(ColorIndex);
		ColorLerp = ColorLerp * ColorLerp * (3.0 - 2.0 * ColorLerp);
		int ColorIndexA = int(ColorIndex) % NumPalette;
		int ColorIndexB = (ColorIndexA + 1) % NumPalette;
		
		Color = lerp(Palette[ColorIndexA], Palette[ColorIndexB], ColorLerp);
	}
	else
	{
		Color = float3(0, 0, 0);
	}
	return Color;
}

[numthreads(8, 8, 1)]
void main(uint3 UTid : SV_DispatchThreadID)
{   
    //float2 NormCoordinates = (float2(UTid.x, UTid.y) + float2(0.5, 0.5)) / Test.Resolution.xy;
    //float2 C = (NormCoordinates - float2(0.5, 0.5)) * 2.0 - float2(1.0, 0.0);
    
    //float2 Z = float2(0, 0);
    //float I = 0;
    //for (I = 0.0; I < 1.0; I += Test.Iter.w) {
    //    Z = float2(
    //        Z.x * Z.x - Z.y * Z.y + C.x,
    //        Z.y * Z.x + Z.x * Z.y + C.y
    //    );
    //    
    //    if (length(Z) > 4.0) {
    //        break;
    //    }
    //}
    
	float2 Coordinates = float2(UTid.x, UTid.y);
    float3 Color = Mandelbrot(Coordinates);//float4(Test.Color.xyz * I, 1.0);
    float4 ToWrite = float4(Color, 1.0);
	ResultImg[UTid.xy] = ToWrite;
}