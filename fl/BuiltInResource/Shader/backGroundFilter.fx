//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
Texture2D maskImageTexture : register(t1);

SamplerState samLinear : register( s0 );

static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
// static const float weight[7] = { 0.03125, 0.109375, 0.21875,0.28125, 0.21875, 0.109375, 0.03125};
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
    // float4 textureShift_1: TEXCOORD1;
    // float4 textureShift_2: TEXCOORD2;
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
    // float2 singleStepOffset = float2(param.y, param.z);
    // output.textureShift_1 = float4(input.Pos - singleStepOffset* 1.182425, input.Pos + singleStepOffset* 1.182425);
    // output.textureShift_2 = float4(input.Pos - 3.029312 * singleStepOffset, input.Pos + 3.029312 * singleStepOffset);
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 oriColor = inputImageTexture.Sample(samLinear, input.Tex);
    float maskface = maskImageTexture.Sample(samLinear, input.Tex).r;
    float2 singleStepOffset = float2(param.y, param.z);
    // float4 sum = float4(0.0,0.0,0.0,0.0);
    // sum += oriColor * 0.398943;
    // sum += inputImageTexture.Sample(samLinear, input.textureShift_1.xy) * 0.295963;
    // sum += inputImageTexture.Sample(samLinear, input.textureShift_1.zw) * 0.295963;
    // sum += inputImageTexture.Sample(samLinear, input.textureShift_2.xy) * 0.004566;
    // sum += inputImageTexture.Sample(samLinear, input.textureShift_2.zw) * 0.004566;

    float4 result = oriColor * weight[0];
    for(int i = 1; i < 5; ++i)
    {
        result += inputImageTexture.Sample(samLinear, input.Tex + singleStepOffset*i) * weight[i];
        result += inputImageTexture.Sample(samLinear, input.Tex - singleStepOffset*i) * weight[i];
    }
    
    result = lerp(oriColor,result,(1.0 - maskface));
    return result;
}  