//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D txMeterial : register(t0);
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  

//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float3 Pos : POSITION;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};  
  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos.xy = input.Pos.xy;
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos.xy*0.5+0.5;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  
float3 ACESFilm(float3 color)
{
    float3 x = 0.8 * color;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
 
float LinearToSrgbChannel(float lin)
{
	if (lin < 0.00313067) return lin * 12.92;
    return pow(lin, (1.0 / 2.4)) * 1.055 - 0.055;
}

float3 LinearToSrgb(float3 lin)
{
	//return pow(lin, 1/2.2);
    return float3(LinearToSrgbChannel(lin.r), LinearToSrgbChannel(lin.g), LinearToSrgbChannel(lin.b));
}

float SrgbToLinearChannel(float lin)
{
	if (lin < 0.04045) return lin/ 12.92;
    return pow(lin / 1.055 + 0.0521327, 2.4);
}


float3 sRGBToLinear(float3 Color)
{
    Color = max(6.10352e-5, Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
    return float3(SrgbToLinearChannel(Color.r), SrgbToLinearChannel(Color.g), SrgbToLinearChannel(Color.b));
}

float3 ToneMapping(float3 Color)
{
    return LinearToSrgb(ACESFilm(Color));
}

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 color = txMeterial.Sample(samLinear,input.Tex);
    return float4(ToneMapping(color.rgb),1.0);
}  