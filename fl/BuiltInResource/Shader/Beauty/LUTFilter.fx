//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
Texture2D inputImageTexture2 : register(t1);

SamplerState samLinear : register( s0 );
  
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
  
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  
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
   float4 textureColor = inputImageTexture.Sample(samLinear,input.Tex);

    float blueColor = textureColor.b * 63.0;
    float2 quad1;
    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);
    float2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);
    float2 texPos1;
    texPos1.x = (quad1.x * 64.0/512.0) + 1.0/512.0 + ((62.0/512.0) * textureColor.r);
    texPos1.y = (quad1.y * 64.0/512.0) + 1.0/512.0 + ((62.0/512.0) * textureColor.g);
    float2 texPos2;
    texPos2.x = (quad2.x * 64.0/512.0) + 1.0/512.0 + ((62.0/512.0) * textureColor.r);
    texPos2.y = (quad2.y * 64.0/512.0) + 1.0/512.0 + ((62.0/512.0) * textureColor.g);
    float4 newColor1 = inputImageTexture2.Sample(samLinear, texPos1);
    float4 newColor2 = inputImageTexture2.Sample(samLinear, texPos2);
    float4 newColor = lerp(newColor1, newColor2, frac(blueColor));

    return lerp(textureColor, float4(newColor.rgb, textureColor.w), param.x);
}  