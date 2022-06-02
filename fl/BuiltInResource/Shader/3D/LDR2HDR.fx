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

  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  
float3 AntiACESFilm(float3 color)
{
    float3 x = color;
    float3 a = -10127.0*x*x+13702.0*x+9.0;
    float3 b = 5.0*pow(a,0.5) + 295.0*x -15.0;
    float3 c = b/(2008.0-1994.0*x);
    return c;
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

float3 AntiToneMapping(float3 Color)
{
    return AntiACESFilm(sRGBToLinear(Color));
}

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 color = txMeterial.Sample(samLinear,input.Tex);
    return float4(AntiToneMapping(color.rgb),1.0);
}  