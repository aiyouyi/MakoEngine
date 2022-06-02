cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
}
Texture2D txDiffuse : register(t0);
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
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float3 W = float3(0.299, 0.587, 0.114);
	float grey = dot(txDiffuse.Sample(samLinear, input.Tex).rgb, W);
	return float4(grey, grey, grey, grey);
};