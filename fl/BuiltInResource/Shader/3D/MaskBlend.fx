Texture2D DstMaterial : register(t0);
Texture2D srcMaterial : register(t1);
SamplerState samLinear : register(s0);


//--------------------------------------------------------------------------------------  
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
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos.xy = input.Pos.xy;
	output.Pos.y = -output.Pos.y;
	output.Pos.z = 0.5;
	output.Pos.w = 1.0;
	output.Tex = input.Pos.xy*0.5 + 0.5;
	return output;
}


//--------------------------------------------------------------------------------------  
// Pixel Shader  
//--------------------------------------------------------------------------------------  

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 dstColor = DstMaterial.Sample(samLinear,input.Tex);
	float4 srcColor = srcMaterial.Sample(samLinear, input.Tex);
	float alpha = srcColor.a;

	float4 color = lerp(dstColor, srcColor, alpha);
	color.a = dstColor.a;

	return color;
}