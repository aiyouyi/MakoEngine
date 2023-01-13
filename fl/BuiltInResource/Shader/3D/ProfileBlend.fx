//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  

Texture2D txMeterial : register(t0);
Texture2D txMeterialMask : register(t1);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
	float2 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float2 Tex1 : TEXCOORD1;

};

//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos.xy = input.Pos.xy * 2.0 - 1.0;
	
	output.Tex1 = input.Pos.xy;
	
	output.Pos.y = -output.Pos.y;
	output.Pos.z = 0.5;
	output.Pos.w = 1.0;
	output.Tex = input.Tex;
	return output;
}


//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 avatarColor = txMeterial.Sample(samLinear,input.Tex);  
	float4 Gcolor = txMeterialMask.Sample(samLinear,input.Tex1);
	
	float alpha = avatarColor.a * Gcolor.a;

	float4 color = lerp(Gcolor,avatarColor,avatarColor.a);

	return float4(color.rgb, alpha);
}