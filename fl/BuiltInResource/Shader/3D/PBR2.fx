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
	float _specularAntiAliasingVariance;
	float _specularAntiAliasingThreshold;
	int bTransparent;
	float AoOffset;
	float4 OutLineColor;
	float4 shadowColor;
	float4 KajiyaSpecularColor; //a代表强度
	float BloomThreshold;
	float BloomStrength;
	int UseEmissiveMask;
	float HDRScale;
};

static const int MAX_MATRICES = 200;
cbuffer ConstantBuffer : register(b1)
{
	matrix BoneMat[MAX_MATRICES];
}

cbuffer ToneMappingParam: register(b2)
{
	int ToneMappingType;
	float ToneMapping_Contrast;
	float ToneMapping_Saturation;
	float PBRPadding2;
}

cbuffer ConstantBuffer2: register(b3)
{
	float HDRContrast;
	float3 PBRPadding1;
}

Texture2D albedoMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughness_metallicMap : register(t2);
Texture2D  EmissMap : register(t3);
Texture2D  AoMap : register(t4);
TextureCube IrradianceTex:register(t5);
Texture2D BrdfLut:register(t6);
TextureCube PrefliterCubeMap:register(t7);
Texture2D ShadowMap : register(t8);
Texture2D PreintegratedSkinLut : register(t9);
Texture2D SkinSpecularBRDF : register(t10);
Texture2D BlurNormalMap : register(t11);
Texture2D ShiftTex: register(t12);
Texture2D OutLineMask: register(t13);
Texture2D EmissiveMask:register(t14);
SamplerState samLinear : register(s0);

static const float HighLightRough = 0.63;
static const float3 HairLight = float3(0.121256,0.130518,0.15625);

static const float PI = 3.14159265359;

#define MIN_ROUGHNESS            0.002025 
static const float anisotropy = 0.8;

#define FLT_EPS            1e-5
#define saturateMediump(x) x

// --------------------------------------------------------------------------

struct ShadingInfo
{
	float3  shading_position;			// position of the fragment in world space
	float3  shading_view;				// normalized vector from the fragment to the eye
	float3  shading_normal;			// normalized transformed normal, in world space
	float3  shading_geometricNormal;  // normalized geometric normal, in world space
	float3  shading_reflected;        // reflection of view about normal
	float shading_NoV;				// dot(normal, view), always strictly >= MIN_N_DOT_V
};

struct MaterialInputs
{
	float4 baseColor;
	float roughness;
	float metallic;
	float specularMask;
	float reflectance;
	float ambientOcclusion;
	float4 emissive;

	float3 normal;
};

struct PixelParams
{
	float3 diffuseColor;
	float perceptualRoughness;
	float perceptualRoughnessUnclamped;
	float3 f0;
	float roughness;
	float3 dfg;
	float3 energyCompensation;
};

struct Light
{
	float4 colorIntensity;	//rgb, pre-exposed intensity
	float3 l;
	float attenuation;
	float NoL;
	float3 worldPosition;
};

#define MIN_N_DOT_V 1e-4

#if defined(TARGET_MOBILE)
    // min roughness such that (MIN_PERCEPTUAL_ROUGHNESS^4) > 0 in fp16 (i.e. 2^(-14/4), rounded up)
    #define MIN_PERCEPTUAL_ROUGHNESS 0.089
    #define MIN_ROUGHNESS            0.007921
#else
    #define MIN_PERCEPTUAL_ROUGHNESS 0.045
    #define MIN_ROUGHNESS            0.002025
#endif

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
	float3 T		: TEXCOORD1;
	float3 B		: TEXCOORD2;
};

struct PS_OUTPUT
{
	float4 Color : SV_Target0;
	float4 Emiss: SV_Target1;
};



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
		PosL.xyz = PosL.xyz + n.xyz*OutlineWidth;
	}

	output.Pos = mul(PosL, ViewProj);

	output.worldPos = PosL.xyz / PosL.w;
	output.Tex = input.Tex;

	output.T = mul(input.Tangent.xyz, (float3x3)ModeltoWorld);
	output.B = cross(input.normal, input.Tangent.xyz) * input.Tangent.w;
	output.B = mul(output.B, (float3x3)ModeltoWorld);

	return output;
}

/*
** Hue, saturation, luminance
*/

float3 RGBToHSL(float3 color)
{
	float3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)

	float fmin = min(min(color.r, color.g), color.b);    //Min. value of RGB
	float fmax = max(max(color.r, color.g), color.b);    //Max. value of RGB
	float delta = fmax - fmin;             //Delta RGB value

	hsl.z = (fmax + fmin) / 2.0; // Luminance

	if (delta == 0.0)		//This is a gray, no chroma...
	{
		hsl.x = 0.0;	// Hue
		hsl.y = 0.0;	// Saturation
	}
	else                                    //Chromatic data...
	{
		if (hsl.z < 0.5)
			hsl.y = delta / (fmax + fmin); // Saturation
		else
			hsl.y = delta / (2.0 - fmax - fmin); // Saturation

		float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
		float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
		float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

		if (color.r == fmax)
			hsl.x = deltaB - deltaG; // Hue
		else if (color.g == fmax)
			hsl.x = (1.0 / 3.0) + deltaR - deltaB; // Hue
		else if (color.b == fmax)
			hsl.x = (2.0 / 3.0) + deltaG - deltaR; // Hue

		if (hsl.x < 0.0)
			hsl.x += 1.0; // Hue
		else if (hsl.x > 1.0)
			hsl.x -= 1.0; // Hue
	}

	return hsl;
}

float HueToRGB(float f1, float f2, float hue)
{
	if (hue < 0.0)
		hue += 1.0;
	else if (hue > 1.0)
		hue -= 1.0;
	float res;
	if ((6.0 * hue) < 1.0)
		res = f1 + (f2 - f1) * 6.0 * hue;
	else if ((2.0 * hue) < 1.0)
		res = f2;
	else if ((3.0 * hue) < 2.0)
		res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
	else
		res = f1;
	return res;
}

float3 HSLToRGB(float3 hsl)
{
	float3 rgb;

	if (hsl.y == 0.0)
		rgb = float3(hsl.z, hsl.z, hsl.z); // Luminance
	else
	{
		float f2;

		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);

		float f1 = 2.0 * hsl.z - f2;

		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0 / 3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b = HueToRGB(f1, f2, hsl.x - (1.0 / 3.0));
	}

	return rgb;
}

float3 specularDFG(const PixelParams pixel)
{
	return lerp( pixel.dfg.xxx, pixel.dfg.yyy, ( 1.0 - pixel.f0 ) );
}

float3 getSpecularDominantDirection(const float3 n, const float3 r, float roughness) {
    return lerp(r, n, roughness * roughness);
}

float3 getReflectedVector(const ShadingInfo shadingInfo,const PixelParams pixel, const float3 n)
{
	float3 r = shadingInfo.shading_reflected;
	return getSpecularDominantDirection(n, r, pixel.roughness);
}

float3 prefilteredRadiance(const float3 r, float perceptualRoughness) {
    float iblRougnessOneLevel = 4.0; //TODO
	float lod = iblRougnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
	float3 R = r;
    R = mul(float4(R, 1.0), RotateIBL).xyz;
	//R.y = -R.y;
	return PrefliterCubeMap.SampleLevel(samLinear, R, lod).rgb*HDRScale;
}

float3 diffuseIrradiance(const float3 n)
{
	return IrradianceTex.Sample(samLinear, n).rgb;
}


//BRDF
float D_GGX(float roughness, float NoH, const float3 h)
{
#if defined(TARGET_MOBILE)
    float3 NxH = cross(shading_normal, h);
    float oneMinusNoHSquared = dot(NxH, NxH);
#else
    float oneMinusNoHSquared = 1.0 - NoH * NoH;
#endif
	
	float a = NoH * roughness;
    float k = roughness / (oneMinusNoHSquared + a * a);
    float d = k * k * (1.0 / PI);
    return saturateMediump(d);
}


float V_SmithGGXCorrelated_Fast(float roughness, float NoV, float NoL)
{
	float v = 0.5 / lerp(2.0 * NoL * NoV, NoL + NoV, roughness);
    return saturateMediump(v);
}

float pow5(float x) 
{
    float x2 = x * x;
    return x2 * x2 * x;
}

float3 F_Schlick(const float3 f0, float f90, float VoH) 
{
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float distribution(float roughness, float NoH, const float3 h) 
{
    return D_GGX(roughness, NoH, h);
}

float visibility(float roughness, float NoV, float NoL) 
{
    return V_SmithGGXCorrelated_Fast(roughness, NoV, NoL);
}

float3 fresnel(const float3 f0, float LoH)
{
    float f90 = clamp(dot(f0, float3(50.0 * 0.33,50.0 * 0.33,50.0 * 0.33)), 0.0, 1.0);
    return F_Schlick(f0, f90, LoH);
}

float3 FlattenNormal(float3 normal,float scale)
{
    return lerp(normal,float3(0.0,0.0,1.0),scale);
}

float3 getFlattenNormal(const VS_OUTPUT input)
{
	float3 tangentNormal = normalMap.Sample(samLinear, input.Tex).xyz * 2.0 - 1.0;
	float3 posOffseted = input.worldPos;
	posOffseted.y += 1.;
	float D = -dot(input.normal, input.worldPos);
	float distToPlane = dot(input.normal, posOffseted) + D;
	float3 proj = posOffseted - input.normal * distToPlane;
	float3 T = normalize(input.T);
	float3 N = normalize(input.normal);
	float3 B = normalize(input.B);
	float3x3 TBN = float3x3(T, B, N);
	float3 finalNormal = mul(tangentNormal, TBN);
    
    finalNormal = FlattenNormal(finalNormal,NormalIntensity);
    float3 Offset = float3(0.0,0.0,1.0) * FrontNormalOffset;
    finalNormal += Offset;

    finalNormal = float3(finalNormal.r,finalNormal.g,finalNormal.b*FrontNoramlScale);
    
    return normalize(finalNormal);
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

float3 ACESToneMapping(float3 Color)
{
	return LinearToSrgb(ACESFilm(Color));
}

//--------------------------------------------------------------------------------------
// AMD Tonemapper
//--------------------------------------------------------------------------------------
// General tonemapping operator, build 'b' term.
float ColToneB(float hdrMax, float contrast, float shoulder, float midIn, float midOut) 
{
    return
        -((-pow(midIn, contrast) + (midOut*(pow(hdrMax, contrast*shoulder)*pow(midIn, contrast) -
            pow(hdrMax, contrast)*pow(midIn, contrast*shoulder)*midOut)) /
            (pow(hdrMax, contrast*shoulder)*midOut - pow(midIn, contrast*shoulder)*midOut)) /
            (pow(midIn, contrast*shoulder)*midOut));
}

// General tonemapping operator, build 'c' term.
float ColToneC(float hdrMax, float contrast, float shoulder, float midIn, float midOut) 
{
    return (pow(hdrMax, contrast*shoulder)*pow(midIn, contrast) - pow(hdrMax, contrast)*pow(midIn, contrast*shoulder)*midOut) /
           (pow(hdrMax, contrast*shoulder)*midOut - pow(midIn, contrast*shoulder)*midOut);
}

// General tonemapping operator, p := {contrast,shoulder,b,c}.
float ColTone(float x, float4 p) 
{ 
    float z = pow(x, p.r); 
    return z / (pow(z, p.g)*p.b + p.a); 
}

float3 AMDTonemapper(float3 color)
{
    static float hdrMax = 16.0; // How much HDR range before clipping. HDR modes likely need this pushed up to say 25.0.
    static float contrast = ToneMapping_Contrast; // Use as a baseline to tune the amount of contrast the tonemapper has.
    static float shoulder = 1.0; // Likely don’t need to mess with this factor, unless matching existing tonemapper is not working well..
    static float midIn = 0.18; // most games will have a {0.0 to 1.0} range for LDR so midIn should be 0.18.
    static float midOut = 0.18; // Use for LDR. For HDR10 10:10:10:2 use maybe 0.18/25.0 to start. For scRGB, I forget what a good starting point is, need to re-calculate.

    float b = ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
    float c = ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

    #define EPS 1e-6f
    float peak = max(color.r, max(color.g, color.b));
    peak = max(EPS, peak);

    float3 ratio = color / peak;
    peak = ColTone(peak, float4(contrast, shoulder, b, c) );
    // then process ratio

    // probably want send these pre-computed (so send over saturation/crossSaturation as a constant)
    float crosstalk = 4.0; // controls amount of channel crosstalk
    float saturation = ToneMapping_Saturation; // full tonal range saturation control
    float crossSaturation = contrast*16.0; // crosstalk saturation

    float white = 1.0;

    // wrap crosstalk in transform
    ratio = pow(abs(ratio), saturation / crossSaturation);
    ratio = lerp(ratio, white, pow(peak, crosstalk));
    ratio = pow(abs(ratio), crossSaturation);

    // then apply ratio to peak
    color = peak * ratio;
    return LinearToSrgb(color);
}


float Luminance(  float3  LinearColor )
{
	return dot( LinearColor,  float3 ( 0.3, 0.59, 0.11 ) );
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
	float3 L = normalize(lightDir[0].xyz);
	float dotValue = abs(dot(normal, L));
	float bias = max(0.01 * (1.0 - dotValue), 0.005);
	return (t - bias <= Moments.x ? 1.0 : pMax);
}

float ComputeShadow(float4 ShadowCoord, float3 Normal)
{
	float3 position = ShadowCoord.xyz / ShadowCoord.w;
	position = position * float3(0.5, -0.5, 0.5) + float3(0.5, 0.5, 0.5);

	float3 Moments = ShadowMap.Sample(samLinear, position.xy).xyz;
	float shadow =  ChebyshevUpperBound(Moments.xy, clamp(position.z, 0.0, 1.0), Normal);
    return 1.0 - (1.0 - shadow) * Moments.z;
	// return shadow * Moments.z;
}

float clampNoV(float NoV) {
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}


//input.worldPos,input.normal
void computeShadingParams(const VS_OUTPUT input,out ShadingInfo shadingInfo)
{
	shadingInfo.shading_normal = getFlattenNormal(input);
	shadingInfo.shading_geometricNormal = normalize(input.normal);
	shadingInfo.shading_position = input.worldPos;
	shadingInfo.shading_view = normalize( CamPos - shadingInfo.shading_position);
	shadingInfo.shading_NoV = clampNoV( dot(shadingInfo.shading_normal, shadingInfo.shading_view) );
	shadingInfo.shading_reflected = reflect( -shadingInfo.shading_view, shadingInfo.shading_normal );
}


void unpremultiply(inout float4 color) {
    color.rgb /= max(color.a, FLT_EPS);
}

float computeDiffuseAlpha(float a)
{
	if (bTransparent == 1)
		return a;		
    return 1.0;
}

float3 computeDiffuseColor(const float4 baseColor, float metallic)
{
	return baseColor.rgb * (1.0 - metallic);
}

float computeDielectricF0(float reflectance) {
    return 0.16 * reflectance * reflectance;
}

float3 computeF0(const float4 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

//Geometric specualr AA
float normalFiltering(float perceptualRoughness, const float3 worldNormal)
{
	float3 du = ddx(worldNormal);
	float3 dv = ddy(worldNormal);

	float variance = _specularAntiAliasingVariance * ( dot(du, du) + dot(dv, dv) );

	float roughness = perceptualRoughnessToRoughness(perceptualRoughness);
	float kernelRoughness = min( 2.0 * variance, _specularAntiAliasingThreshold );
	float squareRoughness = clamp( roughness * roughness + kernelRoughness, 0.0, 1.0 );

	return sqrt( sqrt(squareRoughness) );
}

//IBL
float3 PrefilteredDFG_LUT(float lod, float NoV) {
    // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
    return BrdfLut.Sample(samLinear, float2(NoV, lod)).rgb;
}

float3 prefilteredDFG(float perceptualRoughness, float NoV) {
    // PrefilteredDFG_LUT() takes a LOD, which is sqrt(roughness) = perceptualRoughness
    return PrefilteredDFG_LUT(perceptualRoughness, NoV);
}

void initMaterial(out MaterialInputs material)
{
	material.specularMask = 1.0;
	material.baseColor = float4(1.0,1.0,1.0,1.0);
	material.roughness = 1.0;
	material.metallic = 0.0;
    material.reflectance = 0.5;
	material.ambientOcclusion = 1.0;
	material.emissive = float4(float3(0.0,0.0,0.0), 1.0);
	material.normal = float3(0.0, 0.0, 1.0);
}

void prepareMaterial(const VS_OUTPUT input,const MaterialInputs material,out ShadingInfo shadingInfo)
{
	//shading_normal = normalize( getTBN() * material.normal ); //TODO
	shadingInfo.shading_normal = getFlattenNormal(input);
	shadingInfo.shading_NoV = clampNoV( dot(shadingInfo.shading_normal, shadingInfo.shading_view) );
	shadingInfo.shading_reflected = reflect( -shadingInfo.shading_view, shadingInfo.shading_normal );
}

void setupMaterial(const VS_OUTPUT input,inout MaterialInputs material)
{
	float2 uv = input.Tex;
	material.normal = normalMap.Sample(samLinear, uv ).xyz * 2.0 - 1.0;
    material.baseColor = albedoMap.Sample(samLinear, uv);
	material.baseColor.rgb = sRGBToLinear(pow(material.baseColor.rgb, gamma));
	//material.baseColor.rgb = sRGBToLinear(material.baseColor.rgb);
	if (bTransparent == 1)
		material.baseColor.rgb *= material.baseColor.a;
	int originalTransparent = 0;
	if (material.baseColor.a < 0.01f)
		originalTransparent = 1;
	float4 mr = roughness_metallicMap.Sample(samLinear, uv);
	material.roughness = mr.g;
	material.metallic = mr.b;
	material.specularMask = mr.a;
	material.ambientOcclusion = AoMap.Sample(samLinear, uv).r;
	material.emissive.rgb = EmissMap.Sample(samLinear, uv).rgb;
}


void getPixelParams(const ShadingInfo shadingInfo,const MaterialInputs material, out PixelParams pixel) {

	//getCommonPixelParams
   	float4 baseColor = material.baseColor;
	if (bTransparent == 1)
		unpremultiply(baseColor);
	pixel.diffuseColor = computeDiffuseColor(baseColor, material.metallic);
	float reflectance = computeDielectricF0(material.reflectance);
	pixel.f0 = computeF0( baseColor, material.metallic, reflectance );

	//getRoughnessPixelParams
	float perceptualRoughness = material.roughness;
	pixel.perceptualRoughnessUnclamped = perceptualRoughness;
	perceptualRoughness = normalFiltering( perceptualRoughness, shadingInfo.shading_geometricNormal );

	// Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    pixel.perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = perceptualRoughnessToRoughness(pixel.perceptualRoughness);

	//getEnergyCompensationPixelParams
	pixel.dfg = prefilteredDFG(pixel.perceptualRoughness, shadingInfo.shading_NoV);  //TODO
	pixel.energyCompensation = 1.0 + pixel.f0 * (1.0f / pixel.dfg.y - 1.0f);
	pixel.energyCompensation = float3(1.0f,1.0f,1.0f);
}

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
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

float CenterStep(float sharp,float center,float x)
{
    float s = 1.0 / (1.0 + pow(100000.0, (-3.0 * sharp * (x - center))));
	return s;
}

float3 ApplyKajiya(const VS_OUTPUT input,const ShadingInfo shadingInfo,const MaterialInputs material,const PixelParams pixel)
{
	float3 L = normalize(lightDir[0].xyz);
    float3 H = normalize(shadingInfo.shading_view + L);
	float3 T = normalize( input.T);
	float3 B = normalize( input.B);  
	float NdoH = normalize(dot(shadingInfo.shading_normal,H));
    float GGXValue = D_GGX(NdoH,HighLightRough);

	float4 LightColor0 = float4(lightColors[0].xyz,1.0);
    float4 DiffuseColor = float4(1.0, 1.0, 1.0, 1.0);
    float4 PrimaryColor = float4(1.0, 1.0, 1.0, 1.0);
    float4 SecondaryColor = float4(1.0, 1.0, 1.0, 1.0);
    float4 ambientdiffuse = float4(material.baseColor.rgb,1.0);

	float HairEmiss = clamp(CenterStep(0.5,1.0,GGXValue),0.0,1.0);
	float3 EmissValue = (float3(1.0,1.0,1.0) - material.baseColor.aaa) * HairEmiss * HairLight;

	float4 specular = getSpecular( LightColor0, PrimaryColor, PrimaryShift, SecondaryColor, SecondaryShift, shadingInfo.shading_normal, B, shadingInfo.shading_view, L, SpecularPower, input.Tex*float2(ShiftU,1.0) );
	//specular = specular * material.specularMask * KajiyaSpecularStrength * KajiyaSpecularColor;
	specular *= float4(KajiyaSpecularColor.rgb * KajiyaSpecularColor.a * material.specularMask, KajiyaSpecularColor.a);

	return (ambientdiffuse + specular).rgb + EmissValue;
}

//参数inColor一般是指贴图原来的颜色
//灰度公式是Gray = R*0.299 + G*0.587 + B*0.114。使用dot的方法可以只使用一个指令就完成计算。
float3 GetGray (float3 inColor)
{
	return dot (inColor , float3 (0.3,0.6,0.1));
}

inline float3 GetContrast (float3 inColor)
{
	return (inColor + (GetGray(inColor) -0.5) * HDRContrast);

}

inline float3 GetContrast2(float3 inColor)
{
	float3 grayColor = GetGray(inColor);
	float delta = grayColor - 0.5; // jave.lin : 与 0.5 灰度差距越大，则 对比度强度 越大
	float contrastCol = grayColor + delta * 0.5; // jave.lin : Photoshop 中的对比度，大概在 与0.5灰度差值的 0.5 倍
	return saturate(lerp(inColor, inColor * contrastCol, float3(0.1,0.1,0.1)));
}


void evaluateIBL(const VS_OUTPUT input,const ShadingInfo shadingInfo,const MaterialInputs material, const PixelParams pixel, inout float3 color)
{
	float ssao = 1.0f; //没有ssao
	float diffuseAO = min(material.ambientOcclusion, ssao);
	float specularAO = 1.0f;

	//specular layer
	float3 Fr;
	float3 E = specularDFG(pixel);
	float3 r = getReflectedVector(shadingInfo,pixel, shadingInfo.shading_normal);
	float3 prefilteredColor = prefilteredRadiance(r, pixel.perceptualRoughness);
	Fr = E * GetContrast(prefilteredColor);
	//Fr = E * prefilteredColor;
	Fr *= pixel.energyCompensation;


	float3 iblDiffuse = pixel.diffuseColor;
	if (EnableKajiya == 1)
		iblDiffuse = ApplyKajiya(input,shadingInfo,material, pixel);


	float3 diffuseNormal = shadingInfo.shading_normal;

	float3 diffuseIrradiance_ = diffuseIrradiance(diffuseNormal);
	float3 Fd = iblDiffuse * diffuseIrradiance_ * (1.0 - E) ;

	color.rgb += (Fd + Fr );
	color.rgb *= (diffuseAO+AoOffset);
}

float3 specularLobe( const PixelParams pixel, const Light light, const float3 h,
				float NoV, float NoL, float NoH, float LoH)
{
	float D = distribution(pixel.roughness, NoH, h);
	float V = visibility(pixel.roughness, NoV, NoL);
	float3 F = fresnel(pixel.f0, LoH);

	return (D * V) * F;
}

float diffuse(float roughness, float NoV, float NoL, float LoH)
{
	return 1.0 / PI;
}

float3 diffuseLobe(const PixelParams pixel, float NoV, float NoL, float LoH)
{
	return pixel.diffuseColor * diffuse(pixel.roughness, NoV, NoL, LoH);
}

float3 ChangeShadowColor(float shadowMask, float3 color)
{
	float blendShadowOutput = clamp( shadowMask + shadowColor.a, 0.0,1.0 );
	float3 output_color;
	if (shadowColor.r == 0.0 || shadowColor.r == 1.0)
    {

        output_color = color * blendShadowOutput;
    }
    else
    {
		float3 srcColor = color;
        float3 HSL = RGBToHSL(srcColor.bgr);
        float3 HSL2 = shadowColor.rgb;
        HSL.r += HSL2.r;
		if(HSL.r>1.0)
		{
		    HSL.r-= 1.0;
		}
		HSL.g= lerp( HSL.g,1.0,HSL2.g);
		if(HSL2.b < 0.0)
		{
		    HSL.b= lerp( HSL.b,0.0,-HSL2.b);
		}
		else
		{
		    HSL.b= lerp( HSL.b,1.0,HSL2.b);
		}
        srcColor.bgr = lerp (srcColor.bgr, HSLToRGB(HSL), 1.0);
        output_color = lerp(srcColor, color, blendShadowOutput);        
    }
	return output_color;
}

float3 surfaceShading(const ShadingInfo shadingInfo,const PixelParams pixel, const Light light, float occlusion)
{
	float3 h = normalize(shadingInfo.shading_view + light.l);

	float NoV = shadingInfo.shading_NoV;
	float NoL = clamp( light.NoL, 0.0,1.0 );
	float NoH = clamp( dot(shadingInfo.shading_normal, h), 0.0, 1.0 );
	float LoH = clamp( dot(light.l, h), 0.0, 1.0 );

	float3 Fr = specularLobe(pixel, light, h, NoV, NoL, NoH, LoH);
	float3 Fd = diffuseLobe(pixel, NoV, NoL, LoH);

	float3 color = Fd + (Fr * pixel.energyCompensation);

	color = (color * light.colorIntensity.rgb) *
            (light.colorIntensity.w * light.attenuation * NoL);
	// change shaddow color and intensity
	if (ShadowsEnable)
		color = ChangeShadowColor(occlusion, color);

	return color;
}

Light getDirectionalLight(const ShadingInfo shadingInfo)
{
	Light light;
	light.colorIntensity = float4( lightColors[0].xyz, lightDir[0].w);
	light.l = lightDir[0].xyz;
	light.attenuation = 1.0f;
	light.NoL = clamp( dot( shadingInfo.shading_normal, light.l ), 0.0, 1.0 );
	return light;
}

void evaluateDirectionalLight(const VS_OUTPUT input,const ShadingInfo shadingInfo,const MaterialInputs material,const PixelParams pixel, inout float3 color)
{
	Light light = getDirectionalLight(shadingInfo);
	float visibility = 1.0f;
	if (ShadowsEnable)
		visibility = clamp(ComputeShadow(input.LightPos,normalize(input.normal)),0.0,1.0);

	color.rgb += surfaceShading(shadingInfo,pixel, light, visibility);
}

float4 evaluateLights(const VS_OUTPUT input,const ShadingInfo shadingInfo,const MaterialInputs material)
{
	PixelParams pixel;
	getPixelParams(shadingInfo, material, pixel ); 

	float3 color = float3(0.0,0.0,0.0);

	evaluateIBL(input,shadingInfo,material, pixel, color);

	evaluateDirectionalLight(input,shadingInfo,material, pixel, color);

	if (bTransparent == 1)
		color *= material.baseColor.a;

	return float4( color, computeDiffuseAlpha(material.baseColor.a) );
}

void addEmissive( const MaterialInputs material, inout float4 color )
{
	float4 emissve = material.emissive;
	color.rgb += (emissve.rgb * color.a);
}

float4 evaluateMaterial(const VS_OUTPUT input,const ShadingInfo shadingInfo,const MaterialInputs material)
{
	float4 color = float4(0.0,0.0,0.0,1.0);
	color = evaluateLights(input,shadingInfo,material);
	addEmissive(material, color);
	return color;
}

float3 getBloomColor(const VS_OUTPUT input,float3 color,  const MaterialInputs material)
{
	float EmissMaskValue = 1.0;
	if(UseEmissiveMask == 1)
	{
		EmissMaskValue = EmissiveMask.Sample(samLinear, input.Tex).r;
	}
	float3 out_Emiss = material.emissive.rgb * BloomStrength * EmissMaskValue ;
	return out_Emiss;
}


PS_OUTPUT PS(VS_OUTPUT input) : SV_Target
{
	PS_OUTPUT output;
	
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

	ShadingInfo shadingInfo;
	computeShadingParams(input,shadingInfo);
	
	MaterialInputs material;
	initMaterial(material);

	setupMaterial(input,material);

	float4 out_color = evaluateMaterial(input,shadingInfo,material);
	if (u_EnbleEmiss > 0)
	{
		output.Color = float4(out_color.rgb, out_color.a);

		//float3 emiss = material.emissive.rgb;
		//float EmissStength = 1.0f;
		//emiss = emiss * EmissStength * 10.0;

		//output.Emiss = float4( emiss, 1.0 );
		output.Emiss = float4( getBloomColor(input,out_color.rgb, material), 1.0 );
	}
	else
	{
		if(ToneMappingType == 1)
		{
			output.Color = float4( AMDTonemapper(out_color.rgb), out_color.a );	
		}
		else
		{
			output.Color = float4( ACESToneMapping(out_color.rgb), out_color.a );
		}
		
		output.Emiss = float4(0.f,0.f,0.f,1.f);
	}

	return output;
}