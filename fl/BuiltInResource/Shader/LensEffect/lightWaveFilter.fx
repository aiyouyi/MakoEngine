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
    float4 textureShift_1: TEXCOORD1;
    float4 textureShift_2: TEXCOORD2;
    float4 textureShift_3: TEXCOORD3;
    float4 textureShift_4: TEXCOORD4;
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
    float2 singleStepOffset = float2(param.x, param.y);
    output.textureShift_1 = float4(input.Pos - 2.0 * singleStepOffset, input.Pos + 2.0 *singleStepOffset);
    output.textureShift_2 = float4(input.Pos - 4.0 * singleStepOffset, input.Pos + 4.0 * singleStepOffset);
    output.textureShift_3 = float4(input.Pos - 6.0 * singleStepOffset, input.Pos + 6.0 * singleStepOffset);
    output.textureShift_4 = float4(input.Pos - 8.0 * singleStepOffset, input.Pos + 8.0 * singleStepOffset);
    return output;  
}  
  
float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 iColor = inputImageTexture.Sample(samLinear, input.Tex);

    float4 tmpColor = iColor;
    float4 sum = float4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor =  inputImageTexture.Sample(samLinear, (input.Tex+ input.textureShift_1.xy) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, (input.Tex+ input.textureShift_1.zw) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_1.xy);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_1.zw);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = inputImageTexture.Sample(samLinear, (input.textureShift_1.xy + input.textureShift_2.xy) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, (input.textureShift_1.zw + input.textureShift_2.zw) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_2.xy);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_2.zw);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = inputImageTexture.Sample(samLinear, (input.textureShift_2.xy + input.textureShift_3.xy) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, (input.textureShift_2.zw + input.textureShift_3.zw) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_3.xy);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_3.zw);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = inputImageTexture.Sample(samLinear, (input.textureShift_3.xy + input.textureShift_4.xy) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, (input.textureShift_3.zw + input.textureShift_4.zw) * 0.5);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_4.xy);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = inputImageTexture.Sample(samLinear, input.textureShift_4.zw);
    sum += float4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    if (iColor.a > 0.01)
    {
        sum.rgb /= sum.a;
        sum.a *= 0.05882;
    }
    else
    {
        sum = iColor;
    }

    return sum;
}