Texture2D MainTex : register(t0);
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
    float4 uv01: TEXCOORD1;
    float4 uv23: TEXCOORD2;
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
    float2 mainTex_texelSize = float2(param.y, param.z);
	float2 offset = float2( 1 + param.x, 1 + param.x );

	output.uv01.xy = output.Tex - mainTex_texelSize * offset;
	output.uv01.zw = output.Tex + mainTex_texelSize * offset;
	output.uv23.xy = output.Tex - float2( mainTex_texelSize.x, -mainTex_texelSize.y ) * offset;
	output.uv23.zw = output.Tex + float2( mainTex_texelSize.x, -mainTex_texelSize.y ) * offset;
    return output;  
}  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
	float4 sum = MainTex.Sample(samLinear, input.Tex.xy)*4.0;
	sum +=  MainTex.Sample(samLinear, input.uv01.xy);
	sum +=  MainTex.Sample(samLinear, input.uv01.zw);
	sum +=  MainTex.Sample(samLinear, input.uv23.xy);
	sum +=  MainTex.Sample(samLinear, input.uv23.zw);
	return sum * 0.125;
}