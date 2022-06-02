//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
Texture2D inputImageTexture2 : register(t1);
Texture2D inputImageTextureSkin : register(t2);

SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  

//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float2 Pos : POSITION;
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
    output.Pos.xy = input.Pos*2.0 - float2(1.0,1.0);
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 iColor = inputImageTexture.Sample(samLinear, input.Tex);
    float4 meanColor = inputImageTexture2.Sample(samLinear,  input.Tex);
    float skin = inputImageTextureSkin.Sample(samLinear,  input.Tex).r;
    float3 smoothColor = meanColor.a * iColor.rgb + meanColor.rgb;
    return float4(lerp(iColor.rgb, smoothColor, skin*param.x*0.8), 1.0);

   // return inputImageTexture2.Sample(samLinear,  input.Tex);
}  