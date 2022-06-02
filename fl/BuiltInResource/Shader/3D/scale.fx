Texture2D inputImageTexture : register(t0);
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )  
{  
    float4 param;  
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


//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos.xy = input.Pos.xy;
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos.xy*0.5+0.5;
    return output;  
}  
  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
    float4 iColor = inputImageTexture.Sample(samLinear, input.Tex);
    float2 stepSize = float2(param.y, param.z);

    float4 sum = iColor * 0.5;
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(0.5, 1.0) * stepSize);
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(1.0, -0.5) * stepSize);
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(-0.5, -1.0) * stepSize);
    sum += inputImageTexture.Sample(samLinear, input.Tex+ float2(-1.0, 0.5) * stepSize);


    return float4(sum * 0.22222);
}