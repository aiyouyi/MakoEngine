cbuffer ConstantBuffer : register(b0)
{
    int horizontal;
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
SamplerState samLinear : register(s0);
 
static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

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
    float2 TextureSize;
    InputImage.GetDimensions(TextureSize.x, TextureSize.y);

    float2 tex_offset = 1.0 / TextureSize; // gets size of single texel
    float4 result = InputImage.Sample(samLinear, input.Tex) * weight[0]; // current fragment's contribution
    if(horizontal) 
    {
        for(int i = 1; i < 5; ++i)
        {
            result += InputImage.Sample(samLinear, input.Tex + float2(tex_offset.x * i, 0.0)) * weight[i];
            result += InputImage.Sample(samLinear, input.Tex - float2(tex_offset.x * i, 0.0)) * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += InputImage.Sample(samLinear, input.Tex + float2(0.0, tex_offset.y * i)) * weight[i];
            result += InputImage.Sample(samLinear, input.Tex - float2(0.0, tex_offset.y * i)) * weight[i];
        }
    }
    return result;
}