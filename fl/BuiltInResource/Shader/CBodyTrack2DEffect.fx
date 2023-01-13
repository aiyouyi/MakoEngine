//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
	float4 bgClip;
    float splitNum;
    float currentSplit;
    float2 Affine0_1;
    float4 Affine2_5;
    float2 WithHeight;
    float Alpha;
    float Mirror;
    float4 blendParam;
}
Texture2D txDiffuseEffect : register(t0);
Texture2D txDiffuseMask : register(t1);
Texture2D txSrcMaterial : register(t2);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
    float2 vEffectCoord : TEXCOORD1;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
	
    output.Pos.x = input.Pos.x * Affine0_1.x + input.Pos.y * Affine0_1.y + Affine2_5.x;
    output.Pos.y = input.Pos.x * Affine2_5.y + input.Pos.y * Affine2_5.z + Affine2_5.a;
	
	
    output.vEffectCoord = float2(input.Tex.x*(bgClip.z-bgClip.x) + bgClip.x, input.Tex.y*(bgClip.w-bgClip.y) + bgClip.y);
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

    return abs(blend-base);
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

float4 PS(VS_OUTPUT input) : SV_Target
{
    float2 uv;
    uv.x = input.Pos.x / WithHeight.x;
    uv.y = input.Pos.y / WithHeight.y;
    float2 maskuv=uv;
    maskuv.x -= ((1.0 / splitNum) * currentSplit);
    if (Mirror > 0.5 && currentSplit>0.5)
    {
        maskuv.x = ((1.0 / splitNum) - maskuv.x);
    }
    float mask = txDiffuseMask.Sample(samLinear, maskuv).r;
    float mask1 = txDiffuseMask.Sample(samLinear, maskuv + float2(0, 0.002)).r;
    float mask2 = txDiffuseMask.Sample(samLinear, maskuv + float2(0, -0.002)).r;
    float mask3 = txDiffuseMask.Sample(samLinear, maskuv + float2(0.002, 0)).r;
    float mask4 = txDiffuseMask.Sample(samLinear, maskuv + float2(-0.002, 0.0)).r;
	mask = step(0.5,mask)*3;
	mask += step(0.5,mask1);
	mask += step(0.5,mask2);
	mask += step(0.5,mask3);
	mask += step(0.5,mask4);
	mask/=7.0;

    float4 srcColor = txSrcMaterial.Sample(samLinear, uv);
	float4 dstColor = txDiffuseEffect.Sample(samLinear, input.vEffectCoord);

    if(blendParam.z > 0.5 && dstColor.a > 0.001)
    {
        dstColor.rgb = dstColor.rgb / dstColor.a;
    }

   
    float alpha2 = dstColor.a * blendParam.x;
    float blendtype = blendParam.y;

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
        alpha2 = 1.0;
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

    blendColor.a *= (1.0 - mask);
    blendColor.a *= Alpha;
    if (uv.x < (1.0 / splitNum) * currentSplit || uv.x > (1.0 / splitNum) * (1 + currentSplit))
    {
        blendColor.a *= 0.0;
    }
    return blendColor;
}
