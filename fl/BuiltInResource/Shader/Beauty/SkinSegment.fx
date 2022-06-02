//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);

SamplerState samLinear : register( s0 );
  
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
    output.Pos.xy = input.Pos*2.0 - float2(1.0,1.0);
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos;
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


float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float3 srcColor = inputImageTexture.Sample(samLinear,input.Tex).rgb;
    float3 hsvSpace = rgb2hsv(srcColor.rgb);   //[hue, saturation, value]
    float hue = hsvSpace.x;
    float value = hsvSpace.z;
    float opacity = 1.0;
    float white = srcColor.r + srcColor.g + srcColor.b;
    //skin condition: the range of hue:[0.16,0.18] or [0.89,0.91],the value can not be too low
    if ((0.18 <= hue && hue <= 0.89) || value <= 0.2)
    {
        opacity = 0.0;
    }
    if (0.16 < hue && hue < 0.18)
    {
        opacity = min(opacity, (hue - 0.16) * 50.0);
    }
    if (0.89 < hue && hue < 0.91)
    {
        opacity = min(opacity, (0.91 - hue) * 50.0);
    }
    if (0.2 < value && value < 0.3)
    {
        opacity = min(opacity, (0.3 - value) * 10.0);
    }
    float3 color = float3(opacity,opacity,opacity) * 0.5;
    return float4(clamp(color + 0.5, 0.0, 1.0), 1.0);
}  