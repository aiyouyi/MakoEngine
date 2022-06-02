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
    float4 uv45: TEXCOORD3;
    float4 uv67: TEXCOORD4;
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

	output.uv01.xy = output.Tex + float2( -mainTex_texelSize.x * 2, 0 ) * offset;
	output.uv01.zw = output.Tex + float2( -mainTex_texelSize.x, mainTex_texelSize.y ) * offset;

	output.uv23.xy = output.Tex + float2( 0, mainTex_texelSize.y * 2 ) * offset;
	output.uv23.zw = output.Tex + mainTex_texelSize * offset;

	output.uv45.xy = output.Tex + float2(mainTex_texelSize.x * 2, 0) * offset;
	output.uv45.zw = output.Tex + float2(mainTex_texelSize.x, -mainTex_texelSize.y) * offset;

	output.uv67.xy = output.Tex + float2(0, -mainTex_texelSize.y * 2) * offset;
	output.uv67.zw = output.Tex - mainTex_texelSize * offset;
    return output;  
}  

float4 PS( VS_OUTPUT input ) : SV_Target  
{  
	float4 sum = float4(0,0,0,0);
	sum +=  MainTex.Sample(samLinear, input.uv01.xy);
	sum +=  MainTex.Sample(samLinear, input.uv01.zw)*2.0;
	sum +=  MainTex.Sample(samLinear, input.uv23.xy);
	sum +=  MainTex.Sample(samLinear, input.uv23.zw)*2.0;
	sum +=  MainTex.Sample(samLinear, input.uv45.xy);
	sum +=  MainTex.Sample(samLinear, input.uv45.zw)*2.0;
	sum +=  MainTex.Sample(samLinear, input.uv67.xy);
	sum +=  MainTex.Sample(samLinear, input.uv67.zw)*2.0;

	return sum * 0.0833;
}