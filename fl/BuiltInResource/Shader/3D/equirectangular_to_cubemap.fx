Texture2D EquirectangularMap:register(t0);
SamplerState ClampLinear:register(s0);

cbuffer CBMatrix:register(b0)
{
	matrix View;
	matrix Proj;
    float4 Roughness;
};

struct VertexIn
{
	float3 Pos:POSITION;
};

struct VertexOut
{
	float4 Pos:SV_POSITION;
	float3 LocalPos:POSITION;
};


VertexOut VS(VertexIn ina)
{
	VertexOut outa;
	outa.LocalPos = ina.Pos;
	outa.Pos = mul(float4(ina.Pos, 1.0f), View);
	outa.Pos = mul(outa.Pos, Proj);
	outa.Pos.z = outa.Pos.w;
	return outa;
}


float4 PS(VertexOut outa) : SV_Target
{
    float3 v= normalize(outa.LocalPos);
    float2 uv = float2(atan2(v.z,v.x),asin(v.y));
    uv.x *= 0.1591;
    uv.y *= 0.3183;
	uv += 0.5;
	float3 color = EquirectangularMap.Sample(ClampLinear, uv).rgb;
	return float4(color,1.0);
}