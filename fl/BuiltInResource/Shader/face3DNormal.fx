//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D txNormal : register(t0);
SamplerState samLinear : register( s0 );
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
    matrix matNormal;
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
};  
  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, float4(input.Pos,1.0));
    output.Pos/=output.Pos.w;
	output.Tex = input.Tex;
    output.Tex.y = 1.0-input.Tex.y;
    return output;  
}  
  
  
float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 normal =  txNormal.Sample(samLinear, input.Tex);
	normal = normal*2.0 - 1.0;
	normal = mul(matNormal, normal);
    normal = normal*0.5+0.5; 

   // normal.w = input.Pos.z;
    return normal;
}  