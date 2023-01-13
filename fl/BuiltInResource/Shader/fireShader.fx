//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  

Texture2D noiseTex : register(t0);
Texture2D velocityTex : register(t1);
Texture2D fireTex : register(t2);
Texture2D srcTex : register(t3);
SamplerState samLinear : register(s0);
SamplerState wrapLinear : register(s1);

cbuffer ConstantBuffer : register(b0)
{
    float4 param;  //param.a = width / (float)height  param.b = uTime
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
    float t = fmod(param.y, 1.0);
    float2 uv = input.Tex;
    float2 scaleUV = float2(uv.x, uv.y);
    float ratio = param.x;
    float4 scColor = velocityTex.Sample(samLinear, uv);

    float4 srcColor = srcTex.Sample(samLinear, uv);
    float3 hsvSpace = rgb2hsv(srcColor.rgb);   //[hue, saturation, value]
    float tempValue = 0.01;
    if(hsvSpace.b > threshold.x)
    {
        tempValue =  param.z;
        //return float4(1.0,0.0,0.0,1.0);
    }
    else
    {
        tempValue = param.w;
    }

    float2 noise1 = noiseTex.Sample(wrapLinear, scaleUV * float2(ratio, 1.2) - t * 0.55 ).xy;
    float2 noise = noiseTex.Sample(wrapLinear, scaleUV * float2(ratio, 1.2) + noise1 * 0.065 + t * 0.15 ).xy;

    noise = noise * 2.0 - 1.0;
    float4 newVal = fireTex.Sample(samLinear, uv + noise *tempValue);

    float4 outColor = float4(0.95, 0.85, 1.0, 1.0) * newVal * clamp( noise1.x + 0.5, 0.0, 0.955 );
    float2 noise2 = noiseTex.Sample(wrapLinear, scaleUV * float2(ratio, 1.2) * 5.0 - t * 1.55).xy;

    outColor += scColor * length( noise * noise2 ) * 3.5;
    //outColor.x = min(outColor.x, 0.85);
    //outColor.y = min(outColor.y, 0.75);
    
    return outColor;
}