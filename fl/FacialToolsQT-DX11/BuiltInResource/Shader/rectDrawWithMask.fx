//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
	float4 bgClip;
}
Texture2D txDiffuse : register(t0);
Texture2D txDiffuseEffect : register(t1);
Texture2D txDiffuseMask : register(t2);
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
    float2 vEffectCoord : TEXCOORD1;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
    output.vEffectCoord = float2(input.Tex.x*(bgClip.z-bgClip.x) + bgClip.x, input.Tex.y*(bgClip.w-bgClip.y) + bgClip.y);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float mask = txDiffuseMask.Sample(samLinear, input.Tex).r;
	if(mask <0.5)mask=0.0; 
	else mask=1.0;
	float4 vEffect = txDiffuseEffect.Sample(samLinear, input.vEffectCoord);
	return txDiffuse.Sample(samLinear, input.Tex)*mask + vEffect*(1.0-mask);
}
