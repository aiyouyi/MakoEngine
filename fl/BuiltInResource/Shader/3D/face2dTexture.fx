//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D txMeterial : register(t0);
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  

//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float3 Pos : POSITION;
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
    output.Pos.xy = input.Pos.xy;
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos.xy*0.5+0.5;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 color = txMeterial.Sample(samLinear,input.Tex)*param.x/ 5.0;
    return color;
}  