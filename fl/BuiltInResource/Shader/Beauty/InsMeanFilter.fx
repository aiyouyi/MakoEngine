//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
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
    float texelWidthOffset = param.y;
    float texelHeightOffset = param.z;
    float4 oriColor = inputImageTexture.Sample(samLinear,input.Tex + float2(0.0, 0.0));
    float4 sum = oriColor;
    float4 Color =  inputImageTexture.Sample(samLinear,input.Tex  + float2(-texelWidthOffset,-texelHeightOffset))*0.5;
    sum += Color;
    Color =  inputImageTexture.Sample(samLinear,input.Tex  + float2(texelWidthOffset,texelHeightOffset))*0.5;
    sum += Color;
    Color =  inputImageTexture.Sample(samLinear,input.Tex  + float2(-2.0*texelWidthOffset,-2.0*texelHeightOffset))*0.2;
    sum += Color;
    Color =  inputImageTexture.Sample(samLinear,input.Tex  + float2(2.0*texelWidthOffset,2.0*texelHeightOffset))*0.2;
    sum += Color;
    sum  = sum / 2.4;
    return lerp(oriColor, sum, 1.0 - oriColor.a);
}  