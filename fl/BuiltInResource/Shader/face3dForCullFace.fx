//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
}
struct VS_INPUT
{
	float4 Pos : POSITION;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	return float4(1.0f,1.0f,1.0f,1.0f);
}
