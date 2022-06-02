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
Texture2D txDiffuseEffect : register(t0);
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
	float mask1 = txDiffuseMask.Sample(samLinear, input.Tex+float2(0,0.002)).r;
	float mask2 = txDiffuseMask.Sample(samLinear, input.Tex+float2(0,-0.002)).r;
	float mask3 = txDiffuseMask.Sample(samLinear, input.Tex+float2(0.002,0)).r;
	float mask4 = txDiffuseMask.Sample(samLinear, input.Tex+float2(-0.002,0.0)).r;
	mask = step(0.5,mask)*3;
	mask += step(0.5,mask1);
	mask += step(0.5,mask2);
	mask += step(0.5,mask3);
	mask += step(0.5,mask4);
	mask/=7.0;
	float4 vEffect = txDiffuseEffect.Sample(samLinear, input.vEffectCoord);
	vEffect.a *= (1.0-mask);

	return vEffect;
}
