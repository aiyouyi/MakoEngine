//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
Texture2D inputImageTextureFace : register(t1);
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
    float4 meanColor = float4(0.0,0.0,0.0,0.0);
    float4 corrColor = float4(0.0,0.0,0.0,0.0);
    float WidthOffset = param.y;
    float HeightOffest = param.z;

    float2 textureCoordinate = input.Tex;
    float4 face =  inputImageTextureFace.Sample(samLinear,textureCoordinate);
    float faceAlpha = param.x/0.5;
    float texelWidthOffset = WidthOffset*0.5;
    float texelHeightOffset = HeightOffest*0.5;


    float4 color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(-texelWidthOffset, -texelHeightOffset));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(-texelWidthOffset, 0.0));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(-texelWidthOffset, texelHeightOffset));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(0.0, -texelHeightOffset));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(0.0, 0.0));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;

    float4 SrcColor = color;

    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(0.0, texelHeightOffset));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(texelWidthOffset, -texelHeightOffset));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(texelWidthOffset, 0.0));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = inputImageTexture.Sample(samLinear,textureCoordinate + float2(texelWidthOffset, texelHeightOffset));
    color.a = dot(color.rgb, float3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;

    meanColor *= 0.111111;
    corrColor *= 0.111111;
    float4 varColor = corrColor - meanColor.a * meanColor;
    float a = max(0.0, varColor.a / (varColor.a + faceAlpha * faceAlpha));
    a = 1.0 - (1.0-a)*(face.r);
    float3 b = SrcColor.rgb - a * SrcColor.rgb;
    return float4(b, a);
}  