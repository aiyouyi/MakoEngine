cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
	float4 vColor;
}
Texture2D txDiffuse : register(t0);
Texture2D txDiffuseSrc : register(t1);
Texture2D txDiffuseMask : register(t2);
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
	float2 Tex1 : TEXCOORD1;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
	output.Tex1 = output.Pos.xy *0.5 + 0.5;
	output.Tex1.y = 1.0 - output.Tex1.y;
	return output;
}

float BlendOverlayf(float base, float blend) {
	if(base < 0.5)
	{
		return (2.0 * (base) * (blend));
	}
	else
	{
		return (1.0 - 2.0 * (1.0 - (base)) * (1.0 - (blend)));
	}
    //return (base < 0.5 ? (2.0 * (base) * (blend)) : (1.0 - 2.0 * (1.0 - (base)) * (1.0 - (blend))));
}

float BlendSoftLightf(float base, float blend) {
    return ((1.0 - 2.0 * (blend))*(base)*(base) + 2.0 * (base) * (blend));
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float mask = txDiffuseMask.Sample(samLinear, input.Tex1).r;
	float4 srcColor = txDiffuseSrc.Sample(samLinear, input.Tex1);
    float4 dstColor = txDiffuse.Sample(samLinear, input.Tex);
	float4 blendColor = float4(BlendOverlayf(srcColor.r, dstColor.r), BlendOverlayf(srcColor.g, dstColor.g), BlendOverlayf(srcColor.b, dstColor.b),dstColor.a);
    blendColor = lerp(blendColor,dstColor,mask);
	return blendColor;
};
