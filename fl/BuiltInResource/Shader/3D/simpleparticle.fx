//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  

Texture2D MainTex : register(t0);
SamplerState samLinear : register(s0);


//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
	float3 Pos : POSITION;
	float2 TexCoord : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = float4(input.Pos, 1.0);
	output.Pos.y = -output.Pos.y;
	output.Tex = input.TexCoord;
	return output;
}


//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS(VS_OUTPUT input) : SV_Target
{
	return MainTex.Sample(samLinear,input.Tex);
}