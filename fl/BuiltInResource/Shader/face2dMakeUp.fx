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
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
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
    output.Pos = input.Pos;
    output.Pos.y = -output.Pos.y;
    output.Pos.z=output.Pos.z*0.5+0.5;
    output.Tex = input.Tex;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
   float4 color = txMeterial.Sample(samLinear,input.Tex);
   color.a *= param.x;
   return color;
     
}  