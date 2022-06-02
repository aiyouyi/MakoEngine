//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D txMeterial : register(t0);
Texture2D txMask : register(t1);

SamplerState samLinear : register( s0 );
  
//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float2 TexMask : TEXCOORD1;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 TexMask : TEXCOORD1;
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
    output.TexMask = input.TexMask;
    return output;  
}  
  
  
float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 color2 = txMeterial.Sample(samLinear,input.Tex);
    float4 color3 = txMask.Sample(samLinear,input.TexMask);
    float alpha = color3.r*0.9;//*color3.r;
    color2.a  = color2.a *alpha;
    return color2;
}  