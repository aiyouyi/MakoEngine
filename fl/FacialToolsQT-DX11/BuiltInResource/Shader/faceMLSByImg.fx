//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
//--------------------------------------------------------------------------------------  
// Constant Buffer Variables  
//--------------------------------------------------------------------------------------  
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 transformMatrix;  
}  
Texture2D txDiffuse : register( t0 );
Texture2D txOffest : register( t1 );
SamplerState samLinear : register( s0 );
  
//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 TexOffest : TEXCOORD0;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 TexOffest : TEXCOORD1;
    float2 FaceScale : TEXCOORD2;
};  
  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = float4(input.Pos.xy*2.0-1.0,0.5,1.0);
    output.Pos.y = -output.Pos.y;
    output.Tex = input.Pos.xy;
    output.TexOffest = input.TexOffest;
    output.FaceScale = input.Pos.zw;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  
float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 maskColor = txOffest.Sample(samLinear, input.TexOffest);
    float2 xy = maskColor.rg;//float2(x,y);
    float2 newCoords  = (xy - 0.498) * 0.25;
    newCoords *= input.FaceScale;
    newCoords = float2(dot(newCoords,transformMatrix.rg),dot(newCoords,transformMatrix.ba));

    float4 color = txDiffuse.Sample(samLinear,input.Tex + newCoords);
    return color;

}  