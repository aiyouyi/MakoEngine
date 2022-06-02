//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);

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
    float2 uScanLineJitter; 
    float2 uColorDrift; 
    float intensity;
    float horzIntensity;
    float vertIntensity;
    float uTimeStamp; 
    float4 uHorizontalShake;
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

float nrand(float x, float y)
{
    return frac(sin(dot(float2(x, y), float2(12.9898, 78.233))) * 43758.5453);
}


float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float u = input.Tex.x;
    float v = input.Tex.y;
    float jitter = nrand(v, uTimeStamp) * 2.0 - 1.0;
    jitter *= step(uScanLineJitter.y, abs(jitter)) * uScanLineJitter.x * intensity;
    float jump = lerp(v, frac(v),0);
    float shake = (nrand(uTimeStamp, 2.0) - 0.5) * uHorizontalShake.x;
    float drift = sin(jump + uColorDrift.y) * uColorDrift.x * horzIntensity;

    float4 src1 = inputImageTexture.Sample(samLinear,frac(float2(u + jitter + shake, jump)));
    float4 src2 = inputImageTexture.Sample(samLinear,frac(float2(u + jitter + shake + drift, jump + uColorDrift.y * vertIntensity)));
    src1.g = src2.g;

    return src1;
}  