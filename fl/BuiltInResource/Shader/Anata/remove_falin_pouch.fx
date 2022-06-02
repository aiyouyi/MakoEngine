Texture2D inputImageTexture : register(t0);
Texture2D inputImageTexture2 : register(t1);
Texture2D inputImageTexture3 : register(t2);
Texture2D inputMaskTexture : register(t3);
SamplerState samLinear : register( s0 );
  
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
}  
//------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
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
    output.Tex = (input.Pos.xy + float2(1.0, 1.0)) * 0.5;
    output.Tex1 = input.Tex;
    return output;  
}  
  
  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 color = inputImageTexture.Sample(samLinear, input.Tex);
    float4 maskColor = inputMaskTexture.Sample(samLinear,   input.Tex1);

    if(maskColor.r>0.01&&maskColor.b<0.1)
    {
        float3 resultColor = color.rgb;
        float4 sum = inputImageTexture2.Sample(samLinear,input.Tex);
        float4 sum2 = inputImageTexture3.Sample(samLinear, input.Tex);
        float3 diff1 = clamp((lerp(color.rgb, sum2.rgb, sum.a) - sum.rgb) * 1.3 + 0.03 * sum2.rgb, 0.0, 0.2);
        float3 diff2 = clamp((lerp(color.rgb, sum2.rgb, sum.a) - color.rgb) * 1.3 + 0.03 * sum2.rgb, 0.0, 0.2);
        float3 diffSign = sign(diff1);
        diff1 = diff1 * diffSign;
        diff2 = diff2 * diffSign;
        diff1 = min(diff1, diff2);
        diff1 = max(diff1, diff1 * 0.5);
        diff1 = diff1 * diffSign;

        diff1 = clamp(color.rgb + diff1, 0.0, 1.0);

        color.rgb = lerp(color.rgb, diff1, param.y * maskColor.r * sum.a);
        resultColor = color.rgb;
        return float4(resultColor, 1.0);
    } 

    if(maskColor.b>0.01&&maskColor.r<0.1)//bright eye
    {
        float2 step1 = float2(0.00208, 0.0);
        float2 step2 = float2(0.0, 0.00134);
        float3 sumColor = float3(0.0, 0.0, 0.0);
        for(float t = -2.0; t < 2.5; t += 1.0)
        {
            for(float p = -2.0;p < 2.5; p += 1.0)
            {
                sumColor += inputImageTexture.Sample(samLinear,input.Tex+ t * step1 + p * step2).rgb;
            }
        }
        sumColor = sumColor * 0.04;
        sumColor = clamp(sumColor + (color.rgb - sumColor) * 3.0, 0.0, 1.0);
        sumColor = max(color.rgb, sumColor);
        return float4(lerp(color.rgb, sumColor, maskColor.b*param.z*0.8),color.a);

    }

    if(maskColor.g>0.01)
    {
        float4 sum = inputImageTexture2.Sample(samLinear, input.Tex);
        float4 sum2 = inputImageTexture3.Sample(samLinear, input.Tex);
        float3 imDiff = clamp((lerp(color.rgb, sum2.rgb, sum.a) - sum.rgb) * 1.4 + 0.05 * sum2.rgb, 0.0, 0.3);
        imDiff = min(color.rgb + imDiff, 1.0);
        return float4(lerp(color.rgb,imDiff,param.x * maskColor.g * sum.a),color.a);
    }   
    return color;
}