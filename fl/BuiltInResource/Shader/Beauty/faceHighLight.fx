//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
Texture2D inputImageTexture2 : register(t1);
Texture2D shNormalTexture : register(t2);
Texture2D hightLightLookupTexture : register(t3);
Texture2D shadowLookupTexture : register(t4);

SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  

//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 TexMask : TEXCOORD1;
};  
  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos.xyz = input.Pos;
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos.xy*0.5+float2(0.5,0.5);
    output.TexMask = input.Tex;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float shMask = inputImageTexture2.Sample(samLinear,  input.TexMask).r - 0.5;
    float3 color = inputImageTexture.Sample(samLinear, input.Tex).rgb;
    float3 shNormal = shNormalTexture.Sample(samLinear,  input.TexMask).rgb * 2.0 - float3(1.0,1.0,1.0);

    float3 resShadow;
    float3 resHighlight;
    float rate;
    rate = clamp(dot(param.yzw, shNormal),0.0,1.0);
    rate = rate * rate * param.x * 1.5;
         
    resShadow.r = shadowLookupTexture.Sample(samLinear,  float2(color.r,color.r)).r ;
    resShadow.g = shadowLookupTexture.Sample(samLinear,  float2(color.g,color.g)).r ;
    resShadow.b = shadowLookupTexture.Sample(samLinear,  float2(color.b,color.b)).r ;

    resHighlight.r = hightLightLookupTexture.Sample(samLinear,  float2(color.r,color.r)).r ;
    resHighlight.g = hightLightLookupTexture.Sample(samLinear,  float2(color.g,color.b)).r ;
    resHighlight.b = hightLightLookupTexture.Sample(samLinear,  float2(color.b,color.b)).r ;
         
    resHighlight = lerp(color, resHighlight, max(shMask,0.0));
    resHighlight = lerp(resHighlight, resShadow, max(-shMask,0.0));
    resHighlight = lerp(color, resHighlight, rate);       
    return float4(resHighlight, 1.0);
}  