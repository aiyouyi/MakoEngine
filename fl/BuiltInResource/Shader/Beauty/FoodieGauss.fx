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
    float4 textureShift_1: TEXCOORD1;
    float4 textureShift_2: TEXCOORD2;
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
    float2 singleStepOffset = float2(param.y, param.z);
    output.textureShift_1 = float4(input.Pos - singleStepOffset* 1.182425, input.Pos + singleStepOffset* 1.182425);
    output.textureShift_2 = float4(input.Pos - 3.029312 * singleStepOffset, input.Pos + 3.029312 * singleStepOffset);
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 oriColor = inputImageTexture.Sample(samLinear, input.Tex);
    float4 sum = float4(0.0,0.0,0.0,0.0);
    sum += inputImageTexture.Sample(samLinear, input.Tex) * 0.398943;
    sum += inputImageTexture.Sample(samLinear, input.textureShift_1.xy) * 0.295963;
    sum += inputImageTexture.Sample(samLinear, input.textureShift_1.zw) * 0.295963;
    sum += inputImageTexture.Sample(samLinear, input.textureShift_2.xy) * 0.004566;
    sum += inputImageTexture.Sample(samLinear, input.textureShift_2.zw) * 0.004566;
    return sum;

}  