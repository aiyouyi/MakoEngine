//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  

Texture2D txMaterial : register(t0);
Texture2D txLast : register(t1);
Texture2D bgMask : register(t2);
SamplerState samLinear : register(s0);

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
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos.xy = input.Pos.xy;
    output.Pos.y = -output.Pos.y;
    output.Pos.z = 0.5;
    output.Pos.w = 1.0;
    output.Tex = input.Pos.xy * 0.5 + 0.5;
    return output;
}


//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 color1 = txMaterial.Sample(samLinear,input.Tex);
    float4 color2 = txLast.Sample(samLinear, input.Tex);
    float4 maskColor = bgMask.Sample(samLinear, input.Tex);

    float3 grayW = float3(0.299, 0.587, 0.114);
    float motion = abs( dot( color1.rgb, grayW ) - dot(color2.rgb, grayW) );

    float value = pow(motion, 1.2) * 3.0;
    float3 color = float3( value, value, value );

    float4 resultColor = float4( color1.rgb * color * pow( clamp( maskColor.x * 5.0, 0.0, 1.0 ), 3.0 ), 1.0 );
    return resultColor;
}