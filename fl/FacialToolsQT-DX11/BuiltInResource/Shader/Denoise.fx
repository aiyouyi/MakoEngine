cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
}
Texture2D u_measureImageTexture : register(t0);
Texture2D u_predictionImageTexture : register(t1);
Texture2D u_filterWeightImageTexture : register(t2);
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
	float4 measureColor = u_measureImageTexture.Sample(samLinear, input.Tex); 
	float4 predictColor = u_predictionImageTexture.Sample(samLinear, input.Tex); 
	float fFilterWeight = u_filterWeightImageTexture.Sample(samLinear, input.Tex).y; 
	float4 vRes = predictColor + (measureColor - predictColor)*fFilterWeight; 
	return vRes;
};
