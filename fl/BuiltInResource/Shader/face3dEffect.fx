//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
	matrix matNormal;
	float4 vColor;
}
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color: COLOR; 
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
	output.Color = vColor;
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	return txDiffuse.Sample(samLinear, input.Tex)*input.Color;
}