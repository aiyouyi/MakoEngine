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
    float4 textureShift_3: TEXCOORD3;
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
    output.textureShift_1 = float4(input.Pos - singleStepOffset, input.Pos + singleStepOffset);
    output.textureShift_2 = float4(input.Pos - 2.0 * singleStepOffset, input.Pos + 2.0 * singleStepOffset);
    output.textureShift_3 = float4(input.Pos - 3.0 * singleStepOffset, input.Pos + 3.0 * singleStepOffset);
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float alpha = 0.6;
    float4 oriColor = inputImageTexture.Sample(samLinear, input.Tex);
    float sumMask = 0.28125*2.0;
    float4 sum = oriColor * sumMask;
    
    float4 iColor = inputImageTexture.Sample(samLinear, input.textureShift_1.xy);
    float mask = 0.21875*alpha;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = inputImageTexture.Sample(samLinear, input.textureShift_1.zw);
    sum += iColor * mask;
    sumMask += mask;

    iColor =inputImageTexture.Sample(samLinear, (input.textureShift_1.xy+input.Tex)*0.5);
    mask = 0.21875*alpha*1.5;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = inputImageTexture.Sample(samLinear, (input.textureShift_1.zw+input.Tex)*0.5);
    sum += iColor * mask;
    sumMask += mask;
    

    iColor = inputImageTexture.Sample(samLinear,  (input.textureShift_1.xy+input.textureShift_2.xy)*0.5);
    mask = 0.21875*alpha*0.9;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = inputImageTexture.Sample(samLinear, (input.textureShift_1.zw+input.textureShift_2.zw)*0.5);
    sum += iColor * mask;
    sumMask += mask;

    
    iColor = inputImageTexture.Sample(samLinear,  input.textureShift_2.xy);
    mask = 0.109375*alpha*0.9;
    sum += iColor * mask;
    sumMask += mask;

    iColor = inputImageTexture.Sample(samLinear, input.textureShift_2.zw);
    sum += iColor * mask;
    sumMask += mask;
    


    
    iColor = inputImageTexture.Sample(samLinear,  input.textureShift_3.xy);
    mask = 0.03125*alpha*0.8;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = inputImageTexture.Sample(samLinear, input.textureShift_3.zw);
    sum += iColor * mask;
    sumMask += mask;
    
    sum  = sum / sumMask;

    return lerp(oriColor, sum, 1.0 - oriColor.a);

}  