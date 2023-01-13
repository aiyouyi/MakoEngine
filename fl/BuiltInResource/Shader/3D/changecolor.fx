cbuffer ConstantBuffer : register(b0)
{
    float4 HSVColors;
    int bUseMask;
    float3 pad;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
};
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

Texture2D InputImage : register(t0);
Texture2D MaskTexture : register(t1);
SamplerState samLinear : register(s0);

//É«µ÷×ª»»
float3 RGBtoHCV(float3 rgb) {
    float4 p, q;
    if (rgb.g < rgb.b)
    {
        p = float4(rgb.bg, -1.0, 0.66666667);
    }
    else
    {
        p = float4(rgb.gb, 0.0, -0.33333333);
    }

    if (rgb.g < rgb.b)
    {
        q = float4(p.xyw, rgb.r);
    }
    else
    {
        q = float4(rgb.r, p.yzx);
    }

    float c = q.x - min(q.w, q.y);
    float h = abs((q.w - q.y) / (6.0 * c + 0.001) + q.z);
    return float3(h, c, q.x);
}

float3 RGBtoHSL(float3 rgb) {
    float3 hcv = RGBtoHCV(rgb);
    float l = hcv.z - hcv.y * 0.5;
    float s = hcv.y / (1.0 - abs(l * 2.0 - 1.0) + 0.001);
    return float3(hcv.x, s, l);
}

float3 HSLtoRGB(float3 hsl) {
    float3 rgb;
    float x = hsl.x * 6.0;
    rgb.r = abs(x - 3.0) - 1.0;
    rgb.g = 2.0 - abs(x - 2.0);
    rgb.b = 2.0 - abs(x - 4.0);
    rgb = saturate(rgb);
    float c = (1.0 - abs(2.0 * hsl.z - 1.0)) * hsl.y;
    rgb = saturate((rgb - 0.5) * c + hsl.z);
    return rgb;
}

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

float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 srcColor = InputImage.Sample(samLinear, input.Tex);

    float3 HSL = RGBtoHSL(srcColor.bgr);
    float3 HSL2 = HSVColors.rgb;
    if (bUseMask == 1)
    {
        float mask = MaskTexture.Sample(samLinear, input.Tex).a;
        HSL2 *= mask;
    }
    HSL.r += HSL2.r;
    if (HSL.r > 1.0)
    {
        HSL.r -= 1.0;
    }
    HSL.g = lerp(HSL.g, 1.0, HSL2.g);
    if (HSL2.b < 0.0)
    {
        HSL.b = lerp(HSL.b, 0.0, -HSL2.b);
    }
    else
    {
        HSL.b = lerp(HSL.b, 1.0, HSL2.b);
    }
    srcColor.bgr = lerp(srcColor.bgr, HSLtoRGB(HSL), 1.0);
    return srcColor;
}