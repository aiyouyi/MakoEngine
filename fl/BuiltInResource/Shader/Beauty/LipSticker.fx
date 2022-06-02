//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D inputImageTexture : register(t0);
Texture2D inputImageTexture2 : register(t1);
Texture2D weight : register(t2);
Texture2D LUTTexture : register(t3);

SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )  
{  
    float4 mouthColor;  
    float4 factor;  
    float4 LipRect;
	float4 saturation;
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
    float2 Tex2 : TEXCOORD1;
};  
  
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    float2 LipTex = input.Pos*LipRect.zw+LipRect.xy;

    output.Pos.xy = LipTex*2.0 - float2(1.0,1.0);
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos;
    output.Tex2 = LipTex;
    return output;  
}  
  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  


float BlendOverlayf(float base, float blend)
{
    return (base<0.5?(2.0*(base)*(blend)):(1.0-2.0*(1.0-(base))*(1.0-(blend))));
}

 float3 RGBtoHCV(float3 rgb){
    float4 p,q;
    if(rgb.g<rgb.b)
    {
        p = float4(rgb.bg,-1.0,0.66666667);
    }
    else
    {
        p = float4(rgb.gb,0.0,-0.33333333);
    }

    if(rgb.g<rgb.b)
    {
        q = float4(p.xyw,rgb.r);
    }
    else
    {
        q = float4(rgb.r,p.yzx);
    }

    float c=q.x-min(q.w,q.y);
    float h=abs((q.w-q.y)/(6.0*c+0.001)+q.z);
    return float3(h,c,q.x);
 }
 float3 RGBtoHSL(float3 rgb){
    float3 hcv=RGBtoHCV(rgb);
    float l=hcv.z-hcv.y*0.5;
    float s=hcv.y/(1.0-abs(l*2.0-1.0)+0.001);
    return float3(hcv.x,s,l);
 }
 float3 HSLtoRGB(float3 hsl){
    float3 rgb;
    float x=hsl.x*6.0;
    rgb.r=abs(x-3.0)-1.0;
    rgb.g=2.0-abs(x-2.0);
    rgb.b=2.0-abs(x-4.0);
    rgb=clamp(rgb,0.0,1.0);
    float c=(1.0-abs(2.0*hsl.z-1.0))*hsl.y;
    rgb=clamp((rgb-0.5)*c+hsl.z,0.0,1.0);
    return rgb;
 }
 

float4 PS( VS_OUTPUT input ) : SV_Target  
{  

    float3 source =inputImageTexture.Sample(samLinear,input.Tex2).rgb;
    float4 lipstickMasks=inputImageTexture2.Sample(samLinear,input.Tex);;
    float3 target_color=source;
    if (mouthColor.a>0.001)
    {
        float alpha=lipstickMasks.r*mouthColor.a;
        float contrast_mask=lipstickMasks.r;
        float gray=0.299*source.r+0.587*source.g+0.114*source.b;
        float contrast=max(min((factor.r+factor.g)*contrast_mask+0.5*(1.0-contrast_mask), 1.0),0.0);
        if(contrast<0.5)
        {
            contrast=0.5-(0.5-contrast)*factor.b;
        }

        float blend_weight=weight.Sample(samLinear,input.Tex).r;
        float level_weight=0.7*alpha;
        float3 color=lerp(mouthColor.rgb*source,mouthColor.rgb,blend_weight);
        target_color=lerp(source,color,level_weight);
        float diff=max(gray-1.0,0.0)*alpha;
        target_color=float3(1.0,1.0,1.0)-(float3(1.0,1.0,1.0)-target_color)*(1.0-diff);

        float3 res_color = target_color;
        res_color.r=BlendOverlayf(target_color.r,contrast);
        res_color.g=BlendOverlayf(target_color.g,contrast);
        res_color.b=BlendOverlayf(target_color.b,contrast);
        target_color = lerp(target_color, res_color, alpha);

        float3 HSL = RGBtoHSL(target_color.bgr);
        float3 HSL2 = RGBtoHSL(source.bgr);
        HSL.b = HSL.b*(saturation.x)+HSL2.b*(1.0 - saturation.x);
        target_color.bgr = HSLtoRGB(HSL);
		target_color = lerp(source, target_color, lipstickMasks.r);
    }
    if(factor.a>0.001)
    {
        float blueColor = target_color.b * 15.0;
        float2 quad1;
        quad1.y = floor(floor(blueColor) / 4.0);
        quad1.x = floor(blueColor) - (quad1.y * 4.0);
        float2 quad2;
        quad2.y = floor(ceil(blueColor) / 4.0);
        quad2.x = ceil(blueColor) - (quad2.y * 4.0);
        float2 texPos1;
        texPos1.x = (quad1.x * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.r);
        texPos1.y = (quad1.y * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.g);
        float2 texPos2;
        texPos2.x = (quad2.x * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.r);
        texPos2.y = (quad2.y * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.g);

        float4 newColor1 = LUTTexture.Sample(samLinear, texPos1);
        float4 newColor2 = LUTTexture.Sample(samLinear, texPos2);
        float4 newColor = lerp(newColor1, newColor2, frac(blueColor));
        target_color = lerp(target_color,newColor.rgb, lipstickMasks.g*factor.a);
    }

	return  float4(target_color, 1.0);
}  