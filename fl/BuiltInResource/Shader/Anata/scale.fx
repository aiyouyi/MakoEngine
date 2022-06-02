Texture2D inputImageTexture : register(t0);
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  

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
  
float3 rgb2hsv(float3 rgb)
{
    float4 N = float4(0.0, -0.33333, 0.66667, -1.0);
    float4 f1 = lerp(float4(rgb.bg, N.wz), float4(rgb.gb, N.xy), step(rgb.b, rgb.g));
    float4 f2 = lerp(float4(f1.xyw, rgb.r), float4(rgb.r, f1.yzx), step(f1.x, rgb.r));
    float3 hsv = float3(abs(f2.z + (f2.w - f2.y) / (6.0 * (f2.x - min(f2.w, f2.y)) + 0.0000000001)), 0.0, f2.x);

    return hsv;
}


float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 iColor = inputImageTexture.Sample(samLinear, input.Tex);
    float2 stepSize = float2(param.x, param.y);

    float3 sum = iColor.rgb * 0.5;
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(0.5, 1.0) * stepSize).rgb;
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(1.0, -0.5) * stepSize).rgb;
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(-0.5, -1.0) * stepSize).rgb;
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(-1.0, 0.5) * stepSize).rgb;

    float3 hsvColor = rgb2hsv(iColor.rgb);
    float skinAlpha = clamp((hsvColor.z - 0.2) * 10.0, 0.0, 1.0);
    skinAlpha = skinAlpha * clamp(max((0.18 - hsvColor.x) * 50.0, (hsvColor.x - 0.89) * 50.0), 0.0, 1.0);
    skinAlpha = clamp(skinAlpha + 0.5, 0.0, 1.0);

    return float4(sum * 0.22222, skinAlpha);
}