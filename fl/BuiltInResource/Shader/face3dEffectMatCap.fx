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
Texture2D txMatCap : register(t1);
Texture2D txNormal : register(t2);
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
	float4 normal =  txNormal.Sample(samLinear, input.Tex);
	normal = normal*2.0 - float4(1.0,1.0,1.0,1.0);//
	normal = mul(matNormal, normal);
	float3 N = normalize(normal.xyz);
	float2 tex = N.xy*0.5+ float2(0.5,0.5);
	tex.y = 1.0 - tex.y;
	float4 light = txMatCap.Sample(samLinear,tex);
	float4 color =  txDiffuse.Sample(samLinear, input.Tex)*input.Color;
	float gray = dot(light.rgb, float3(0.2989, 0.5870, 0.1140));
	color.rgb = color.rgb*light*1.2;//*2.0*0.7 ;//+ color *0.5; 
	

	return color;
}