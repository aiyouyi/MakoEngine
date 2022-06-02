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
    float2 Pos : POSITION;
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
    //output.Pos.xy = input.Pos * 2.0 - float2(1.0, 1.0);
    //output.Pos.y = -output.Pos.y;
    output.Pos.xy = input.Pos;
    output.Pos.y = -output.Pos.y;
    output.Pos.z = 0.5;
    output.Pos.w = 1.0;
    output.Tex = (input.Pos + float2(1, 1)) * 0.5;
    //output.Tex = input.Pos;
    //
    float cx = cos(param.x);
    float sx = sin(param.x);
    output.Pos.xy = output.Pos.xy * float2(cx, cx) + output.Pos.yx * float2(0 - sx, sx);
    output.Pos.y *= param.y;
    //
    if (param.z > 2.5)
    {
        output.Pos.x = -output.Pos.x;
        output.Pos.y = -output.Pos.y;
    }
    else if (param.z > 1.5)
    {
        output.Pos.x = -output.Pos.x;        
    }
    else if (param.z > 0.5)
    {
        output.Pos.y = -output.Pos.y;
    }
    //
    //        
        return output;
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float2 SplitU(float2 Tex,float4 param)
{
    float2 uv = Tex;
    float w = frac(param.w / 10) * 10;
    uv.x = frac(uv.x * w) / w + 0.5 - 0.5 / w;
    float b = floor(param.w / 10);
    float c = 0.5 - abs(uv.x - 0.5);
    float d = 0.5 + abs(uv.x - 0.5);
    float cc = (param.z > 1.5) ? 1 : 0;
    c = d * cc + c * (1-cc);
    uv.x = uv.x * (1 - b) + c * b;
    return uv;
}

float2 SplitV(float2 Tex, float4 param)
{
    float2 uv = Tex;
    float w = frac(param.w / 10) * 10;
    uv.y = frac(uv.y * w) / w + 0.5 - 0.5 / w;
    float b = floor(param.w / 10);
    float c = 0.5 - abs(uv.y - 0.5);
    float d = 0.5 + abs(uv.y - 0.5);
    float cc = (param.z > 1.5) ? 1 : 0;
    c = d * cc + c * (1 - cc);
    uv.y = uv.y * (1 - b) + c * b;
    return uv;
}

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float2 uv = (abs(param.x) > 0.5) ? SplitV(input.Tex, param) : SplitU(input.Tex, param);
    float4 color = txMeterial.Sample(samLinear, uv);
    return color;
}  