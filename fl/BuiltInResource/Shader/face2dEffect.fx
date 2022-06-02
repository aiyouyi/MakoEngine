//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D txMeterial : register(t0);
Texture2D txMask : register(t1);
Texture2D srcMeterial : register(t2);

SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
    float4 matrixParam;  
} 

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
    float2 Tex1 : TEXCOORD2;
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
    output.Tex.x = input.Tex.x * param.z + input.Tex.y * param.a +  matrixParam.x;
    output.Tex.y = input.Tex.x * matrixParam.y + input.Tex.y * matrixParam.z + matrixParam.a;
    output.TexMask = input.TexMask;
    output.Tex1 = (input.Pos.xy + float2(1.0, 1.0)) * 0.5;
    return output;  
}  
  
//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  
//柔光
float blendSoftLight(float base, float blend)
{
    float color1 = 2.0 * base * blend + base * base * (1.0 - 2.0 * blend);
    float color2 = sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend);
    return lerp(color1, color2, step(0.5, blend));
}
//滤色
float BlendScreenf(float base, float blend) {
    return (1.0 - ((1.0 - (base)) * (1.0 - (blend))));
}

//变暗
float BlendDarkenf(float base, float blend) {
    return min(blend, base);
}
//颜色加深
float BlendColorBurnf(float base, float blend) {
    return ((blend == 0.0) ? blend : max((1.0 - ((1.0 - (base)) / (blend))), 0.0));
}
//线性加深
float BlendLinearBurnf(float base, float blend) {
    return max(base + blend - 1.0, 0.0);
}
//深色

//变亮
float BlendLightenf(float base, float blend) {
    return max(blend, base);
}
//颜色减淡
float BlendColorDodgef(float base, float blend) {
    return ((blend == 1.0) ? blend : min((base) / (1.0 - (blend)), 1.0));
}
//线性减淡
float BlendLinearDodgef(float base, float blend) {
    return min(base + blend, 1.0);
}
//深色
float3 BlendDarkerColor(float3 base, float3 blend)
{
    if(length(base) < length(blend))
    {
        return base;
    }
    else
    {
        return blend;
    }
}
//浅色
float3 BlendLighterColor(float3 base, float3 blend)
{
    if(length(base) > length(blend))
    {
        return base;
    }
    else
    {
        return blend;
    }

}
//叠加
float BlendOverlayf(float base, float blend) {
    return (base < 0.5 ? (2.0 * (base) * (blend)) : (1.0 - 2.0 * (1.0 - (base)) * (1.0 - (blend))));
}

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    // float4 color2 = txMeterial.Sample(samLinear,input.Tex);
    // float4 color3 = txMask.Sample(samLinear,input.TexMask);
    // float alpha = color3.r;//*color3.r;
    // color2.a  = color2.a *alpha;
    // return color2;

    float4 srcColor = srcMeterial.Sample(samLinear,input.Tex1);
   
    
    if(input.Tex.x > 1 || input.Tex.x < 0)
    {
        return srcColor;
    }
    if(input.Tex.y > 1 || input.Tex.y < 0)
    {
         return srcColor;
    }
    
    float4 dstColor = txMeterial.Sample(samLinear,input.Tex);
    float4 mask = txMask.Sample(samLinear,input.TexMask);

    float alpha2 = mask.r * dstColor.a * param.x;
    float blendtype = param.y;

    float4 blendColor = dstColor;
    if(blendtype == 1.0)
    {
        blendColor = srcColor * dstColor;
        blendColor.a = alpha2;
    }
    else if(blendtype == 2.0)
    {
        blendColor = float4(blendSoftLight(srcColor.r, dstColor.r),blendSoftLight(srcColor.g, dstColor.g),blendSoftLight(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 3.0)
    {
        blendColor = float4(BlendScreenf(srcColor.r, dstColor.r),BlendScreenf(srcColor.g, dstColor.g),BlendScreenf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 4.0)
    {
        blendColor = float4(BlendDarkenf(srcColor.r, dstColor.r),BlendDarkenf(srcColor.g, dstColor.g),BlendDarkenf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 5.0)
    {
        blendColor = float4(BlendColorBurnf(srcColor.r, dstColor.r),BlendColorBurnf(srcColor.g, dstColor.g),BlendColorBurnf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 6.0)
    {
        blendColor = float4(BlendLinearBurnf(srcColor.r, dstColor.r),BlendLinearBurnf(srcColor.g, dstColor.g),BlendLinearBurnf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 7.0)
    {
        blendColor = float4(BlendDarkerColor(srcColor.rgb, dstColor.rgb),alpha2);
    }
    else if(blendtype == 8.0)
    {
        blendColor = float4(BlendLightenf(srcColor.r, dstColor.r),BlendLightenf(srcColor.g, dstColor.g),BlendLightenf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 9.0)
    {
        blendColor = float4(BlendColorDodgef(srcColor.r, dstColor.r),BlendColorDodgef(srcColor.g, dstColor.g),BlendColorDodgef(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 10.0)
    {
        blendColor = float4(BlendLinearDodgef(srcColor.r, dstColor.r),BlendLinearDodgef(srcColor.g, dstColor.g),BlendLinearDodgef(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 11.0)
    {
        blendColor = float4(BlendLighterColor(srcColor.rgb, dstColor.rgb),alpha2);
    }
    else if(blendtype == 12.0)
    {
        blendColor = float4(BlendOverlayf(srcColor.r, dstColor.r),BlendOverlayf(srcColor.g, dstColor.g),BlendOverlayf(srcColor.b, dstColor.b),alpha2);
    }

    blendColor = lerp(srcColor,blendColor,alpha2);
    blendColor.a=1.0;

    return blendColor;
}  