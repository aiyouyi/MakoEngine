//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
	matrix VP;
}
Texture2D txDiffuse : register(t0);
Texture2D txMatCap : register(t1);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 N : NORMAL;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 N : NORMAL;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, float4(input.Pos,1.0));
	output.N = mul(VP, input.N).xyz;
	output.Tex = input.Tex;
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float2 tex = input.N.xy*0.5+ float2(0.5,0.5);
	float4 color =  txDiffuse.Sample(samLinear, input.Tex);

	float4 light = txMatCap.Sample(samLinear,tex);
    float gray = dot(light.rgb, float3(0.2989, 0.5870, 0.1140));
	float4 result = color*0.5+ gray*1.2*0.5*(color); 
	result.a = 1.0;
	return result;//float4(tex,1.0,1.0);//result;//float4(gray,gray,gray,1.0);
}