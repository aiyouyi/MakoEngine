//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  

cbuffer ConstantBuffer : register(b0)
{
	matrix world;
	matrix meshMat;
	matrix view;
	matrix projection;
	matrix meshMatInverse;
	matrix RotateIBL;
	matrix lightSpaceMatrix;
	float4 lightDir[4];
	float4 lightColors[4];
	int LightNum;
	float3 CamPos;
	float ambientStrength;
	float RoughnessRate;
	int AnimationEnable;
	int ReverseY;
	int ShadowsEnable;
	float gamma;
	int u_EnbleRMGamma;
	int u_EnbleEmiss;
	float NormalIntensity;
	float FrontNoramlScale;
	float FrontNormalOffset;
	int   EnableKajiya;
	float PrimaryShift;
	float SecondaryShift;
 	float SpecularPower;
 	float ShiftU;
 	float KajiyaSpecularScale;
 	float KajiyaSpecularWidth;
	int EnableRenderOutLine;
	float OutlineWidth;
	float4 OutLineColor;
};
Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughness_metallicMap : register(t2);
Texture2D  EmissMap : register(t3);
TextureCube IrradianceTex:register(t4);
TextureCube PrefliterCubeMap:register(t5);
Texture2D BrdfLut:register(t6);
Texture2D ShadowMap : register(t7);
Texture2D  AoMap : register(t8);
Texture2D ShiftTex: register(t9);
Texture2D OutLineMask: register(t10);
SamplerState samLinear : register(s0);


static const float PI = 3.14159265359;
struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4 Tangent: TANGENT;
	float4 BlendIndices  : BLENDINDICES;
	float4 BlendWeights  : BLENDWEIGHT;
	

};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 normal: NORMAL;
	float4 LightPos : POSITION0;
	float3 worldPos	: POSITION1;
	float2 Tex : TEXCOORD0;
	float4 Tangent	: TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 Color : SV_Target0;
	float4 Emiss: SV_Target1;
};

static const int MAX_MATRICES = 200;
cbuffer ConstantBuffer : register(b1)
{
	matrix BoneMat[MAX_MATRICES];
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	matrix ModeltoWorld = { 1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 1.0, 0.0,
							0.0, 0.0, 0.0, 1.0 };
	if (AnimationEnable == 1)
	{
		int i0 = int(input.BlendIndices.x);
		int i1 = int(input.BlendIndices.y);
		int i2 = int(input.BlendIndices.z);
		int i3 = int(input.BlendIndices.w);
		matrix BoneTransform = BoneMat[i0] * input.BlendWeights.x;
		BoneTransform += BoneMat[i1] * input.BlendWeights.y;
		BoneTransform += BoneMat[i2] * input.BlendWeights.z;
		BoneTransform += BoneMat[i3] * input.BlendWeights.w;

		ModeltoWorld = mul(meshMatInverse, BoneTransform);
	}

	ModeltoWorld = mul(meshMat, ModeltoWorld);
	if (ShadowsEnable)
	{
		float4 PosS = mul(float4(input.Pos, 1.0), ModeltoWorld);
		output.LightPos = mul(PosS, lightSpaceMatrix);
	}

	ModeltoWorld = mul(ModeltoWorld, world);

	//matrix ModeltoWorld=mul(BoneTransform, world);

	float4 PosL = mul(float4(input.Pos, 1.0), ModeltoWorld);
	matrix ViewProj = mul(view, projection);

	float4 n = float4(input.normal, 0.0);
	n = mul(n, ModeltoWorld);
	float4 ns = float4(0.0, 0.0, 0.0, 0.0);
	ns = mul(ns, ModeltoWorld);
	output.normal = normalize(n - ns).xyz;

	if(EnableRenderOutLine)
	{
		PosL.xyz = PosL.xyz + n*OutlineWidth;
	}

	output.Pos = mul(PosL, ViewProj);

	output.worldPos = PosL.xyz / PosL.w;
	output.Tex = input.Tex;


	float4 tan = mul(float4( input.Tangent.xyz, 0.0),ModeltoWorld);
	output.Tangent = float4( tan.xyz, input.Tangent.w );
	return output;
}

float3 getNormalFromMap(VS_OUTPUT input)
{
	float3 tangentNormal = normalMap.Sample(samLinear, input.Tex).xyz * 2.0 - 1.0;
	float3 Q1 = normalize(ddx(input.worldPos));
	float3 Q2 = normalize(ddy(input.worldPos));
	float2 st1 = ddx(input.Tex);
	float2 st2 = ddy(input.Tex);

	float3 N = normalize(input.normal);
	float3 T = Q1 * st2.y - Q2 * st1.y;

	if (length(T) < 0.00001)
	{
		return N;
	}
	T = normalize(T);
	float3 B = -normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);
	float3 finalNormal = mul(tangentNormal, TBN);
	return normalize(finalNormal);
}

float3 getNormalFromMapEXT(VS_OUTPUT input)
{
	float3 tangentNormal = normalMap.Sample(samLinear, input.Tex).xyz * 2.0 - 1.0;
	float3 posOffseted = input.worldPos;
	posOffseted.y += 1.;
	float D = -dot(input.normal, input.worldPos);
	float distToPlane = dot(input.normal, posOffseted) + D;
	float3 proj = posOffseted - input.normal * distToPlane;
	float3 T = normalize(proj - input.worldPos);
	float3 N = normalize(input.normal);
	float3 B = -normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);
	float3 finalNormal = mul(tangentNormal, TBN);
	return normalize(finalNormal);
}
// google filament扒下来的
// 高光项
float D_GGX(float NoH, float a) {
	float a2 = a * a;
	float f = (NoH * a2 - NoH) * NoH + 1.0;
	return a2 / (PI * f * f);
}

float3 F_Schlick(float VoH, float3 f0) {
	return f0 + (float3(1.0, 1.0, 1.0) - f0) * pow(1.0 - VoH, 5.0);
}

float F_Schlick(float f0, float f90, float VoH) {
	return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float tmp = 1.0 - roughness;
	return F0 + (max(float3(tmp, tmp, tmp), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
	float a2 = a * a;
	float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
	float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
	return 0.5 / (GGXV + GGXL);
}

float V_SmithGGXCorrelated_Fast(float NoV, float NoL, float a) {
	// Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
	float v = 0.5 / lerp(2.0 * NoL * NoV, NoL + NoV, a);
	return v;
}
// clearCoat的V项 简化计算 其余两项还是使用之前的
float V_Kelemen(float LoH) {
	return 0.25 / (LoH * LoH);
}

// 漫反项
float Fd_Lambert() {
	return 1.0 / PI;
}

float3 BRDF(float3 n, float3 v, float3 l, float NoL, float roughness, float metalic, float3 diffuseColor, float3 radiance, float  g_alpha) {
	float3 h = normalize(v + l);

	float NoV = abs(dot(n, v)) + 1e-5;
	float NoH = clamp(dot(n, h), 0.0, 1.0);
	float LoH = clamp(dot(l, h), 0.0, 1.0);
	// perceptually linear roughness (see parameterization)
	// remapping
	float a = roughness * roughness;
	float D = D_GGX(NoH, a);

	float alpha = clamp(D, 0.0, 1.0);
	g_alpha = alpha * (1.0 - g_alpha);

	float3 diffuse = diffuseColor * (1.0 - g_alpha) + radiance * g_alpha;

	float reflectance = 0.5;
	float3  g_f0 = 0.16 * reflectance * reflectance * (1.0 - metalic) + diffuse * metalic;
	float3  F = F_Schlick(LoH, g_f0);
	float V = V_SmithGGXCorrelated_Fast(NoV, NoL, a);
	float3 Fr = (D * V) * F;

	float3 kD = 1.0 - F;
	kD *= 1.0 - metalic;
	float3 Fd = kD * diffuse * Fd_Lambert();
	float3 brdfLobe = Fd + Fr;
	return brdfLobe;
}

float3 ACESFilm(float3 color)
{
	float3 x = 0.8 * color;
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float LinearToSrgbChannel(float lin)
{
	if (lin < 0.00313067) return lin * 12.92;
	return pow(lin, (1.0 / 2.4)) * 1.055 - 0.055;
}

float3 LinearToSrgb(float3 lin)
{
	//return pow(lin, 1/2.2);
	return float3(LinearToSrgbChannel(lin.r), LinearToSrgbChannel(lin.g), LinearToSrgbChannel(lin.b));
}

float SrgbToLinearChannel(float lin)
{
	if (lin < 0.04045) return lin / 12.92;
	return pow(lin / 1.055 + 0.0521327, 2.4);
}


float3 sRGBToLinear(float3 Color)
{
	Color = max(6.10352e-5, Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
	return float3(SrgbToLinearChannel(Color.r), SrgbToLinearChannel(Color.g), SrgbToLinearChannel(Color.b));
}

float3 ToneMapping(float3 Color)
{
	return LinearToSrgb(ACESFilm(Color));
}

float Linstep(float a, float b, float v)
{
	return clamp((v - a) / (b - a), 0.0, 0.8);
}
// Reduces VSM light bleedning
float ReduceLightBleeding(float pMax, float amount)
{
	// Remove the [0, amount] tail and linearly rescale (amount, 1].
	return Linstep(amount, 1.0f, pMax);
}

float ChebyshevUpperBound(float2 Moments, float t, float3 Normal)
{
	float Variance = Moments.y - Moments.x * Moments.x;
	float MinVariance = 0.0000001;
	Variance = max(Variance, MinVariance);

	// Compute probabilistic upper bound.
	float d = t - Moments.x;
	float pMax = Variance / (Variance + d * d);

	float lightBleedingReduction = 0.5;
	pMax = ReduceLightBleeding(pMax, lightBleedingReduction);

	pMax /= 0.8;
	float3 normal = normalize(Normal);
	float3 lightDir2 = -lightDir[0].xyz;
	lightDir2.z = -lightDir2.z;
	float dotValue = abs(dot(normal, lightDir2));
	float bias = max(0.01 * (1.0 - dotValue), 0.005);
	return (t - bias <= Moments.x ? 1.0 : pMax);
}

float ComputeShadow(float4 ShadowCoord, float3 Normal)
{
	float3 position = ShadowCoord.xyz / ShadowCoord.w;
	position = position * float3(0.5, -0.5, 0.5) + float3(0.5, 0.5, 0.5);

	float2 Moments = ShadowMap.Sample(samLinear, position.xy).xy;
	return ChebyshevUpperBound(Moments, clamp(position.z, 0.0, 1.0), Normal);
}

float3 FlattenNormal(float3 normal,float scale)
{
    return lerp(normal,float3(0.0,0.0,1.0),scale);
}

float3 getFlattenNormal(VS_OUTPUT input)
{
	float3 tangentNormal = normalMap.Sample(samLinear, input.Tex).xyz * 2.0 - 1.0;
	float3 posOffseted = input.worldPos;
	posOffseted.y += 1.;
	float D = -dot(input.normal, input.worldPos);
	float distToPlane = dot(input.normal, posOffseted) + D;
	float3 proj = posOffseted - input.normal * distToPlane;
	float3 T = normalize(proj - input.worldPos);
	float3 N = normalize(input.normal);
	float3 B = -normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);
	float3 finalNormal = mul(tangentNormal, TBN);
    
    finalNormal = FlattenNormal(finalNormal,NormalIntensity);
    float3 Offset = float3(0.0,0.0,1.0) * FrontNormalOffset;
    finalNormal += Offset;

    finalNormal = float3(finalNormal.r,finalNormal.g,finalNormal.b*FrontNoramlScale);
    
    return normalize(finalNormal);
}

float3 shiftTangent(float3 T, float3 N, float shift)
{
    return normalize(T + shift * N);
}

float hairStrand(float3 T, float3 V, float3 L, float specPower)
{
    float3 H = normalize(V + L);

    float HdotT = dot(T, H);
    float sinTH = sqrt(1 - HdotT * HdotT);
    float dirAtten = smoothstep(-KajiyaSpecularWidth, 0, HdotT);

    return dirAtten * saturate(pow(sinTH, specPower)) * KajiyaSpecularScale;
}


float4 getSpecular(float4 lightColor0,
    float4 primaryColor, float primaryShift,
    float4 secondaryColor, float secondaryShift,
    float3 N, float3 T, float3 V, float3 L, float specPower, float2 uv)
{
    float shiftTex = ShiftTex.Sample(samLinear, uv).r - 0.5;

    float3 t1 = shiftTangent(T, N, primaryShift + shiftTex);
    float3 t2 = shiftTangent(T, N, secondaryShift + shiftTex);

    float4 specular = float4(0.0, 0.0, 0.0, 0.0);
    specular += primaryColor * hairStrand(t1, V, L, specPower) * KajiyaSpecularScale;
    specular += secondaryColor * hairStrand(t2, V, L, specPower) * KajiyaSpecularScale;

    return specular;
}

PS_OUTPUT PS(VS_OUTPUT input) : SV_Target
{
	PS_OUTPUT output;
	float4 srcColor = albedoMap.Sample(samLinear, input.Tex);
	//float3 albedo     = pow(srcColor.rgb, 2.2);
	float3 albedo = sRGBToLinear(pow(srcColor.rgb, gamma));
	
	if(EnableRenderOutLine)
	{
		float albedoMask = OutLineMask.Sample(samLinear, input.Tex).r;
		if(albedoMask < 0.5f)
			discard;
		//output.Color = lerp(float4(albedo,1.0),OutLineColor,albedoMask);
		output.Color = OutLineColor;
		output.Emiss = float4(0.0,0.0,0.0,1.0);
		return output;
	}

	float4 mr = roughness_metallicMap.Sample(samLinear, input.Tex);
	if (u_EnbleRMGamma > 0)
	{
		mr.g = pow(mr.g, 1.0 / 2.2);
	}
	float metallic = mr.b;
	float roughness = max(mr.g,0.001) * RoughnessRate;

	float ao = AoMap.Sample(samLinear, input.Tex).r;
	float3 N = getFlattenNormal(input);
    
	float3 V = normalize(CamPos - input.worldPos);
	float3 F0 = float3(0.04,0.04,0.04);
	F0 = lerp(F0, albedo, metallic);

	if(EnableKajiya == 1)
	{
		float4 LightColor0 = float4(1.0, 1.0, 1.0, 1.0);
        float4 PrimaryColor = float4(1.0, 1.0, 1.0, 1.0);
        float4 SecondaryColor = float4(1.0, 1.0, 1.0, 1.0);
        float4 ambientdiffuse = float4(albedo,1.0);
		
		float3 T = normalize( input.Tangent.xyz);
	    float3 B = normalize( cross(N, T) * sign(input.Tangent.w));

        float4 specular = getSpecular(LightColor0, PrimaryColor, PrimaryShift, SecondaryColor, SecondaryShift, N, normalize(B), V, lightDir[0].xyz, SpecularPower, input.Tex*float2(ShiftU,1.0));

        albedo = (ambientdiffuse + specular).rgb;
	}

	// reflectance equation
	float3 Lo = float3(0.0,0,0);
	for (int i = 0; i < LightNum; ++i)
	{
		// calculate per-light radiance
		float3 L = normalize(lightDir[i].xyz);
		float3 H = normalize(V + L);
		float3 radiance = lightColors[i].xyz;

		float NdotL = max(dot(N, L), 0.0);
		//float NdotL = dot(N, L)*0.5+0.5;      

		float3 BRDFterm = BRDF(N,V,L, NdotL, roughness, metallic, albedo,radiance,srcColor.a);

		Lo += BRDFterm * radiance * NdotL *lightDir[i].w;
	}
    //N.y = -N.y;
	float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	float3 R = reflect(-V, N);
	R = mul(float4(R, 1.0), RotateIBL).xyz;
	float3 kD = 1.0 - F;
	kD *= 1.0 - metallic;
	if (ReverseY == 1)
	{
		R.y = -R.y;
	}

	float3 irradiance = IrradianceTex.Sample(samLinear, N).xyz;
	const float MAX_REF_LOD = 7.0;
	float3 prefliterColor = PrefliterCubeMap.SampleLevel(samLinear, R, MAX_REF_LOD * roughness).rgb;
	float3 brdf = BrdfLut.Sample(samLinear, float2(max(dot(N, V), 0.0), roughness)).xyz;
	float3 iblSpecular = prefliterColor * (F * brdf.x + brdf.y);
	float3 iblDiffuse = irradiance * albedo * kD;
	float3 ambient = iblDiffuse + iblSpecular;
	float3 color = ambientStrength * ambient;


	if (ShadowsEnable)
	{
		float shadowCoef = clamp(ComputeShadow(input.LightPos, normalize(input.normal)), 0.0, 1.0);
		color += Lo * (shadowCoef);
	}
	else
	{
		color += Lo;
	}
	if (u_EnbleEmiss > 0)
	{
		float3 emiss = sRGBToLinear(EmissMap.Sample(samLinear, input.Tex).rgb);
		output.Color = float4(ao*color,srcColor.a);
		float3 emiss_color = emiss * 10.0;
		output.Emiss = float4(emiss_color,1.0);
	}
	else
	{
		output.Color = float4(ToneMapping(ao*color),srcColor.a);
		output.Emiss = float4(0.0,0.0,0.0,1.0);
	}

	return output;
}