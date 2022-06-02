TextureCube HdrCubeMap:register(t0);
SamplerState TrilinearFliterClamp:register(s0);
static const float PI = 3.1415926;


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
	float3 SkyPos:TEXCOORD0;
};


VertexOut VS(VertexIn ina)
{
	VertexOut outa;
	outa.SkyPos = ina.Pos;
	outa.Pos = float4(ina.Pos, 1.0f);
	outa.Pos = mul(outa.Pos, View);
	outa.Pos = mul(outa.Pos, Proj);
	outa.Pos.z = outa.Pos.w;
	return outa;
}

float DistributionGGX(float3 n, float3 h, float roughness)
{
	float a2 = roughness * roughness;
	float nDoth = max(dot(n, h), 0.0);
	float nDoth2 = nDoth * nDoth;

	float denom = (nDoth2 * (a2 - 1.0) + 1.0);
	denom = max(PI * denom * denom, 0.001);

	return a2 / denom;
}

float RadicalInverse_Vdc(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_Vdc(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}


float4 PS(VertexOut outa) : SV_Target
{
	float3 N = normalize(outa.SkyPos);
	float3 R = N;
	float3 V = R;

	const uint SAMPLE_COUNT = 1024;
	float3 perfilteredColor = float3(0.0, 0.0, 0.0);
	float totalWeight = 0.0;

	for (uint i = 0; i < SAMPLE_COUNT; ++i)
	{
		// generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, Roughness.x);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			float D = DistributionGGX(N, H, Roughness.x);
			float NdotH = max(dot(N, H), 0.0);
			float HdotV = max(dot(H, V), 0.0);
			float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

			float resolution = 512.0; // resolution of source cubemap (per face)
			float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
			float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

			float mipLevel = Roughness.x == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
			perfilteredColor += HdrCubeMap.SampleLevel(TrilinearFliterClamp, L, mipLevel).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	perfilteredColor /= totalWeight;
	return float4(perfilteredColor, 1.0);
}