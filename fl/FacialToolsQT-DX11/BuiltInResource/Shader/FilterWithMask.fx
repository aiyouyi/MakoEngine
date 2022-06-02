//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
}
Texture2D txDiffuse : register(t0);
Texture2D txDiffuseMask : register(t1);
SamplerState samLinear : register(s0);
struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float mask = txDiffuseMask.Sample(samLinear, input.Tex).r;
	if(mask <0.5)mask=0.0; 
	else mask=1.0;
	return txDiffuse.Sample(samLinear, input.Tex)*mask ;
}
