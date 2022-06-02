//--------------------------------------------------------------------------------------  
//   

//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  

  
//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 OffestColor : COLOR;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};  
  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = input.Pos*2.0-1.0;
    output.Pos.y = -output.Pos.y;
    output.Color = input.OffestColor;//float4(input.Pos.x,0,0.0,1.0);
    return output;  
}  
  
  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    return input.Color;
}  