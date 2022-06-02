#ifndef PI
#define PI 3.1415926
#endif

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

float GeometrySchlickGGXForIBL(float NdotV, float roughness)
{
	// note that we use a different k for IBL
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmithForIBL(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGXForIBL(NdotV, roughness);
	float ggx1 = GeometrySchlickGGXForIBL(NdotL, roughness);

	return ggx1 * ggx2;
}

float2 IntegrateBRDF(float NDotV, float roughness)
{
	float3 V;
	V.x = sqrt(1.0 - NDotV * NDotV);
	V.y = 0.0;
	V.z = NDotV;

	float A = 0.0;
	float B = 0.0;

	float3 N = float3(0.0, 0.0, 1.0);
	const uint SAMPLE_COUNT = 1024;

	for (uint i = 0; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(L.z, 0.0);
		float NdotH = max(H.z, 0.0);
		float VdotH = max(dot(V, H), 0.0);

		if (NdotL > 0)
		{
			float G = GeometrySmithForIBL(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NDotV);
			float FC = pow(1.0 - VdotH, 5.0);
			A += (1.0 - FC) * G_Vis;
			B += FC * G_Vis;
		}
	}

	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);

	return float2(A, B);
}

struct VertexIn2
{
	float3 Pos:POSITION;
};

struct VertexOut2
{
	float4 Pos:SV_POSITION;
	float2 Tex:TEXCOORD0;
};

VertexOut2 VS(VertexIn2 ina)
{
	VertexOut2 outa;
	outa.Pos = float4(ina.Pos.xy, 1.0, 1.0);
	outa.Tex = ina.Pos.xy*0.5+0.5;
	return outa;
}

float4 PS(VertexOut2 outa) : SV_Target
{
	float2 color2 =  IntegrateBRDF(outa.Tex.x, 1.0-outa.Tex.y);
	return float4(color2, 1.0, 1.0);
}