//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  

Texture2D txMaterial : register(t0);
Texture2D fireTex : register(t1);
Texture2D bgMask : register(t2);
SamplerState samLinear : register(s0);

cbuffer ConstBuffer : register(b0)
{
    float4 clip;
    float4 threshold;
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

float3 rgb2hsv(float3 c)
{
    float4 K = float4(0.0, - 0.33333, 0.66667, -1.0);
    float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
    float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    float s = 0.0; //for optimization, set0.0 right result: d / (q.x + e)
    float3 hsv = float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)),s , q.x);
    return hsv;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    float2 uv = input.Tex;
    float2 maskUV = uv;
    maskUV.x -= ((1.0 / clip.x) * clip.y);
    float4 color1 = txMaterial.Sample(samLinear, uv);
    float4 color2 = fireTex.Sample(samLinear, maskUV);
    float mask = bgMask.Sample(samLinear, maskUV).r;

    float3 hsvSpace = rgb2hsv(color1.rgb);   //[hue, saturation, value]

    float tempValue = 0.55;
    if(hsvSpace.b > threshold.x)
    {
        tempValue = clip.z;
        //return float4(1.0,0.0,0.0,1.0);
    }
    else
    {
        tempValue = clip.w;
    }

    //float mask = bgMask.Sample(samLinear, maskUV).r;
    //float mask1 = bgMask.Sample(samLinear, maskUV + float2(0, 0.002)).r;
    //float mask2 = bgMask.Sample(samLinear, maskUV + float2(0, -0.002)).r;
    //float mask3 = bgMask.Sample(samLinear, maskUV + float2(0.002, 0)).r;
    //float mask4 = bgMask.Sample(samLinear, maskUV + float2(-0.002, 0.0)).r;
    //mask = step(0.5, mask) * 3;
    //mask += step(0.5, mask1);
    //mask += step(0.5, mask2);
    //mask += step(0.5, mask3);
    //mask += step(0.5, mask4);
    //mask /= 7.0;

    float4 outColor = color1 + color2 * color2 * tempValue * (1.0 - pow(clamp(mask * 1.1, 0.0, 1.0), 1.35));
    if (uv.x < (1.0 / clip.x) * clip.y || uv.x >(1.0 / clip.x) * (1 + clip.y))
    {
        outColor.a *= 0;
    }

    return outColor;
}