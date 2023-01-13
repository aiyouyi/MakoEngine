#version 300 es

#ifdef GL_ES//for discriminate GLES & GL
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision highp float;
#endif
#else
#define highp
#define mediump
#define lowp
#endif

in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec4 Tangent;

layout (location = 0) out vec4 out_FragColor;
layout (location = 1) out vec4 out_EmissColor;

//texture map material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughness_metallicMap;
uniform sampler2D EmissMap;
uniform sampler2D aoMap;

//shadow map
uniform sampler2D shadowMap;
//whether to render shadow  暂时先写在这里，后面看如何进行内存对齐
uniform bool shadowsEnable;
uniform vec4 shadowColor;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform sampler2D PreintegratedSkinLut;
uniform sampler2D SkinSpecularBRDF;
uniform sampler2D BlurNormalMap;
uniform sampler2D shiftMap;
uniform sampler2D OutLineMask;
uniform mat4 RotateIBL;
// lights
uniform vec4 lightDir[4];
uniform vec4 lightColors[4];
uniform float lightStrength[4];
uniform int LightNum;
uniform float ambientStrength;
uniform float EmissStength;
uniform float gamma;
uniform int  EnbleRMGamma;
uniform vec3 EmissAddColor;
uniform float BloomThreshold;
uniform float BloomStrength;
uniform int u_EnbleEmiss;
uniform vec3 camPos;
uniform int materialType;
uniform bool EnableSkinRender;
uniform vec3 SubsurfaceColor;
uniform float SkinSmooth;
uniform float SkinSpecularScale;
uniform float SkinCurveFactor;
uniform vec3 TuneNormalBlur;
uniform int bUseBlurNormal;
uniform int bUseHairColor;
uniform vec3 HairColorHSV;

uniform  float NormalIntensity;
uniform float FrontNormalOffset;
uniform float FrontNoramlScale;
uniform int bUseKajiya;
uniform float SecondaryShift;
uniform float PrimaryShift;
uniform float SpecularPower;
uniform float ShiftU;
uniform float KajiyaSpecularScale;
uniform float KajiyaSpecularWidth;
uniform float KajiyaSpecularStrength;
uniform vec4 KajiyaSpecularColor;
uniform int EnableRenderOutLine;
uniform vec4 OutlineColor;
uniform int bTransparent;
uniform float AoOffset;

uniform float _specularAntiAliasingVariance;	// 0.15
uniform float _specularAntiAliasingThreshold;	//0.04
uniform vec2 padding;

const float PI = 3.14159265359;
float g_alpha = 1.0;

#define MIN_ROUGHNESS            0.002025 
float anisotropy = 0.8;

#define FLT_EPS            1e-5
#define saturateMediump(x) x

const float HighLightRough = 0.63;
const vec3 HairLight = vec3(0.121256,0.130518,0.15625);
// --------------------------------------------------------------------------

vec3  shading_position;			// position of the fragment in world space
vec3  shading_view;				// normalized vector from the fragment to the eye
vec3  shading_normal;			// normalized transformed normal, in world space
vec3  shading_geometricNormal;  // normalized geometric normal, in world space
vec3  shading_reflected;        // reflection of view about normal
float shading_NoV;				// dot(normal, view), always strictly >= MIN_N_DOT_V

struct MaterialInputs
{
	vec4 baseColor;
	float roughness;
	float metallic;
	float specularMask;
	float reflectance;
	float ambientOcclusion;
	vec4 emissive;

	vec3 normal;
};

struct PixelParams
{
	vec3 diffuseColor;
	float perceptualRoughness;
	float perceptualRoughnessUnclamped;
	vec3 f0;
	float roughness;
	vec3 dfg;
	vec3 energyCompensation;
};

struct Light
{
	vec4 colorIntensity;	//rgb, pre-exposed intensity
	vec3 l;
	float attenuation;
	float NoL;
	vec3 worldPosition;
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

float clampNoV(float NoV) {
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
    return max(NoV, MIN_N_DOT_V);
}

vec3 computeDiffuseColor(const vec4 baseColor, float metallic)
{
	return baseColor.rgb * (1.0 - metallic);
}

float computeDielectricF0(float reflectance) {
    return 0.16 * reflectance * reflectance;
}

vec3 computeF0(const vec4 baseColor, float metallic, float reflectance) {
    return baseColor.rgb * metallic + (reflectance * (1.0 - metallic));
}

float perceptualRoughnessToRoughness(float perceptualRoughness) {
    return perceptualRoughness * perceptualRoughness;
}

mat3 getTBN()
{
	vec3 n = normalize( Normal);
	vec3 t = normalize( Tangent.xyz);
	vec3 b = normalize( cross(n, t) * sign(Tangent.w));
	mat3 tbn = mat3(t, b, n);
	return tbn;
}

vec3 FlattenNormal(vec3 normal,float scale)
{
    return mix(normal,vec3(0.0,0.0,1.0),scale);
}

vec3 getFlattenNormal()
{
	vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
	vec3 posOffseted = WorldPos;
    posOffseted.y += 1.;
    float D = -dot(Normal, WorldPos);
    float distToPlane = dot(Normal, posOffseted) + D;
    vec3 proj = posOffseted - Normal * distToPlane;
    vec3 T = normalize(proj - WorldPos);
	vec3 N = normalize(Normal);
	vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    vec3 finalNormal = TBN * tangentNormal;
    
    finalNormal = FlattenNormal(finalNormal,NormalIntensity);
    vec3 Offset = vec3(0.0,0.0,1.0) * FrontNormalOffset;
    finalNormal += Offset;

    finalNormal = vec3(finalNormal.r,finalNormal.g,finalNormal.b*FrontNoramlScale);
    
    return normalize(finalNormal);
}

vec3 getNormalFromMapEXT()
{
	vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
	vec3 posOffseted = WorldPos;
    posOffseted.y += 1.;
    float D = -dot(Normal, WorldPos);
    float distToPlane = dot(Normal, posOffseted) + D;
    vec3 proj = posOffseted - Normal * distToPlane;
    vec3 T = normalize(proj - WorldPos);
	vec3 N = normalize(Normal);
	vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    vec3 finalNormal = normalize(TBN * tangentNormal);
    return finalNormal;
}

void unpremultiply(inout vec4 color) {
    color.rgb /= max(color.a, FLT_EPS);
}

float computeDiffuseAlpha(float a)
{
	if (bTransparent == 1)
		return a;		
    return 1.0;
}

//Geometric specualr AA
float normalFiltering(float perceptualRoughness, const vec3 worldNormal)
{
	vec3 du = dFdx(worldNormal);
	vec3 dv = dFdy(worldNormal);

	float variance = _specularAntiAliasingVariance * ( dot(du, du) + dot(dv, dv) );

	float roughness = perceptualRoughnessToRoughness(perceptualRoughness);
	float kernelRoughness = min( 2.0 * variance, _specularAntiAliasingThreshold );
	float squareRoughness = clamp( roughness * roughness + kernelRoughness, 0.0, 1.0 );

	return sqrt( sqrt(squareRoughness) );
}

float Linstep(float a, float b, float v)
{
	return clamp((v - a) / (b - a),0.0,0.8);
}

// Reduces VSM light bleedning
float ReduceLightBleeding(float pMax, float amount)
{
	// Remove the [0, amount] tail and linearly rescale (amount, 1].
	return Linstep(amount, 1.0f, pMax);
}

float ChebyshevUpperBound(vec2 Moments, float t) 
{
	float Variance = Moments.y - Moments.x * Moments.x;
	float MinVariance = 0.0000001;
	Variance = max(Variance, MinVariance);

	// Compute probabilistic upper bound.
	float d = t - Moments.x;
	float pMax = Variance / (Variance + d * d);

	float lightBleedingReduction = 0.5;
	pMax = ReduceLightBleeding(pMax, lightBleedingReduction);
    
    pMax /=0.8;
     vec3 normal = normalize(Normal);
    float dotValue = abs(dot(normal, -lightDir[0].xyz));
    float bias = max(0.01 * (1.0 - dotValue), 0.005);
	return (t-bias <= Moments.x ? 1.0 : pMax);
}

float ComputeShadow(vec4 ShadowCoord)
{
	vec3 position = ShadowCoord.xyz / ShadowCoord.w;
    position = position * 0.5 + 0.5;

	vec3 Moments = texture(shadowMap, position.xy).xyz;
    float shadow = ChebyshevUpperBound(Moments.xy, clamp(position.z,0.0,1.0));

	return 1.0-(1.0-shadow)*Moments.z;
}

  /*
** Hue, saturation, luminance
*/

vec3 RGBToHSL(vec3 color)
{
	vec3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)
	
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

		if (color.r == fmax )
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

vec3 HSLToRGB(vec3 hsl)
{
	vec3 rgb;
	
	if (hsl.y == 0.0)
		rgb = vec3(hsl.z); // Luminance
	else
	{
		float f2;
		
		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
			
		float f1 = 2.0 * hsl.z - f2;
		
		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b= HueToRGB(f1, f2, hsl.x - (1.0/3.0));
	}
	
	return rgb;
}

vec3 ChangeShadowColor(float shadowMask, vec3 color)
{
	float blendShadowOutput = clamp( shadowMask + shadowColor.a, 0.0,1.0 );
	vec3 output_color;
	if (shadowColor.r == 0.0 || shadowColor.r == 1.0)
    {

        output_color = color * blendShadowOutput;
    }
    else
    {
		vec3 srcColor = color;
        vec3 HSL = RGBToHSL(srcColor.bgr);
        vec3 HSL2 = shadowColor.rgb;
        HSL.r += HSL2.r;
		if(HSL.r>1.0)
		{
		    HSL.r-= 1.0;
		}
		HSL.g= mix( HSL.g,1.0,HSL2.g);
		if(HSL2.b < 0.0)
		{
		    HSL.b= mix( HSL.b,0.0,-HSL2.b);
		}
		else
		{
		    HSL.b= mix( HSL.b,1.0,HSL2.b);
		}
        srcColor.bgr = mix (srcColor.bgr, HSLToRGB(HSL), 1.0);
        output_color = mix(srcColor, color, blendShadowOutput);        
    }
	return output_color;
}

float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

float CenterStep(float sharp,float center,float x)
{
    float s = 1.0 / (1.0 + pow(100000.0, (-3.0 * sharp * (x - center))));
	return s;
}

vec3 shiftTangent(vec3 T, vec3 N, float shift)
{
    return normalize(T + shift * N);
}

float hairStrand(vec3 T, vec3 V, vec3 L, float specPower)
{
    vec3 H = normalize(V + L);

    float HdotT = dot(T, H);
    float sinTH = sqrt(1.0 - HdotT * HdotT);
    float dirAtten = smoothstep(-KajiyaSpecularWidth, 0.0, HdotT);

    return dirAtten * clamp(pow(sinTH, specPower),0.0,1.0) * KajiyaSpecularScale;
}

vec4 getSpecular(vec4 lightColor0,
    vec4 primaryColor, float primaryShift,
    vec4 secondaryColor, float secondaryShift,
    vec3 N, vec3 T, vec3 V, vec3 L, float specPower, vec2 uv)
{
    float ShiftTex = texture(shiftMap, uv).r - 0.5;

    vec3 t1 = shiftTangent(T, N, primaryShift + ShiftTex);
    vec3 t2 = shiftTangent(T, N, secondaryShift + ShiftTex);

    vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);
    specular += primaryColor * hairStrand(t1, V, L, specPower) * KajiyaSpecularScale;
    specular += secondaryColor * hairStrand(t2, V, L, specPower) * KajiyaSpecularScale;

    return specular;
}

vec3 ApplyKajiya(const MaterialInputs material,
        const PixelParams pixel)
{
	vec3 L = normalize(-lightDir[0].xyz);
    vec3 H = normalize(shading_view + L);
	vec3 T = normalize( Tangent.xyz);
	vec3 B = normalize( cross(shading_normal, T) * sign(Tangent.w));  //用shading_normal 还是shading_geometricNormal
	float NdoH = normalize(dot(shading_normal,H));
    float GGXValue = D_GGX(NdoH,HighLightRough);

	vec4 LightColor0 = vec4(lightColors[0].xyz,1.0);
    vec4 DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 PrimaryColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 SecondaryColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 ambientdiffuse = vec4(material.baseColor.rgb,1.0);

	float HairEmiss = clamp(CenterStep(0.5,1.0,GGXValue),0.0,1.0);
	vec3 EmissValue = (vec3(1.0,1.0,1.0) - material.baseColor.aaa) * HairEmiss * HairLight;

	vec4 specular = getSpecular( LightColor0, PrimaryColor, PrimaryShift, SecondaryColor, SecondaryShift, shading_normal, normalize(B), shading_view, L, SpecularPower, TexCoords*vec2(ShiftU,1.0) );
	specular = specular * material.specularMask * KajiyaSpecularStrength * KajiyaSpecularColor;

	return (ambientdiffuse + specular).rgb + EmissValue;
}

//IBL
vec3 PrefilteredDFG_LUT(float lod, float NoV) {
    // coord = sqrt(linear_roughness), which is the mapping used by cmgen.
    return textureLod(brdfLUT, vec2(NoV, lod), 0.0).rgb;
}

vec3 prefilteredDFG(float perceptualRoughness, float NoV) {
    // PrefilteredDFG_LUT() takes a LOD, which is sqrt(roughness) = perceptualRoughness
    return PrefilteredDFG_LUT(perceptualRoughness, NoV);
}

vec3 specularDFG(const PixelParams pixel)
{
	return mix( pixel.dfg.xxx, pixel.dfg.yyy, ( 1.0 - pixel.f0 ) );
}

vec3 getSpecularDominantDirection(const vec3 n, const vec3 r, float roughness) {
    return mix(r, n, roughness * roughness);
}

vec3 getReflectedVector(const PixelParams pixel, const vec3 n)
{
	vec3 r = shading_reflected;
	return getSpecularDominantDirection(n, r, pixel.roughness);
}

vec3 prefilteredRadiance(const vec3 r, float perceptualRoughness) {
    float iblRougnessOneLevel = 4.0; //TODO
	float lod = iblRougnessOneLevel * perceptualRoughness * (2.0 - perceptualRoughness);
	vec3 R = r;
	R.y = -R.y;
    return textureLod(prefilterMap, R, lod).rgb;	//TODO
}

//vec3 Irradiance_SphericalHarmonics(const vec3 n)
//{
//	    return max(
//          iblSH[0]
//        + iblSH[1] * (n.y)
//        + iblSH[2] * (n.z)
//        + iblSH[3] * (n.x)
//
//        + iblSH[4] * (n.y * n.x)
//        + iblSH[5] * (n.y * n.z)
//        + iblSH[6] * (3.0 * n.z * n.z - 1.0)
//        + iblSH[7] * (n.z * n.x)
//        + iblSH[8] * (n.x * n.x - n.y * n.y)
//
//        , 0.0);
//}

vec3 diffuseIrradiance(const vec3 n)
{
	vec3 negN = n;
	negN.y = -negN.y;
	return texture(irradianceMap, negN).rgb;
}

//vec3 diffuseIrradiance(const vec3 n)
//{
//	return Irradiance_SphericalHarmonics(n);
//}

//BRDF
float D_GGX(float roughness, float NoH, const vec3 h)
{
#if defined(TARGET_MOBILE)
    vec3 NxH = cross(shading_normal, h);
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
	float v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, roughness);
    return saturateMediump(v);
}

float pow5(float x) 
{
    float x2 = x * x;
    return x2 * x2 * x;
}

vec3 F_Schlick(const vec3 f0, float f90, float VoH) 
{
    // Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
    return f0 + (f90 - f0) * pow5(1.0 - VoH);
}

float distribution(float roughness, float NoH, const vec3 h) 
{
    return D_GGX(roughness, NoH, h);
}

float visibility(float roughness, float NoV, float NoL) 
{
    return V_SmithGGXCorrelated_Fast(roughness, NoV, NoL);
}

vec3 fresnel(const vec3 f0, float LoH)
{
    float f90 = clamp(dot(f0, vec3(50.0 * 0.33)), 0.0, 1.0);
    return F_Schlick(f0, f90, LoH);
}

void computeShadingParams()
{
	shading_geometricNormal = normalize(Normal);
	shading_position = WorldPos;
	shading_view = normalize( camPos - shading_position);
}

vec3 ACESFilm(vec3 color)
{
	vec3 x = 0.8 * color;
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x*(a*x + b)) / (x*(c*x + d) + e),0.0,1.0);
}
 
float LinearToSrgbChannel(float lin)
{
	if (lin < 0.00313067) return lin * 12.92;
	return pow(lin, (1.0 / 2.4)) * 1.055 - 0.055;
}

vec3 LinearToSrgb(vec3 lin)
{
	//return pow(lin, 1/2.2);
	return vec3(LinearToSrgbChannel(lin.r), LinearToSrgbChannel(lin.g), LinearToSrgbChannel(lin.b));
}

float SrgbToLinearChannel(float lin)
{
	if (lin < 0.04045) return lin/ 12.92;
	return pow(lin / 1.055 + 0.0521327,2.4);
}


vec3 sRGBToLinear(vec3 Color)
{
	Color = max(vec3(6.10352e-5), Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
    return vec3(SrgbToLinearChannel(Color.r), SrgbToLinearChannel(Color.g), SrgbToLinearChannel(Color.b));
}

vec3 ToneMapping(vec3 Color)
{
	return LinearToSrgb(ACESFilm(Color));
}

float Luminance(  vec3  LinearColor )
{
	return dot( LinearColor,  vec3 ( 0.3, 0.59, 0.11 ) );
}

void initMaterial(out MaterialInputs material)
{
	material.baseColor = vec4(1.0);
	material.roughness = 1.0;
	material.metallic = 0.0;
    material.reflectance = 0.5;
	material.ambientOcclusion = 1.0;
	material.emissive = vec4(vec3(0.0), 1.0);
	material.normal = vec3(0.0, 0.0, 1.0);
}

void prepareMaterial(const MaterialInputs material)
{
	//shading_normal = normalize( getTBN() * material.normal ); //TODO
	shading_normal = getFlattenNormal();
	shading_NoV = clampNoV( dot(shading_normal, shading_view) );
	shading_reflected = reflect( -shading_view, shading_normal );
}

void material(inout MaterialInputs material)
{
	vec2 uv = TexCoords;
	material.normal = texture( normalMap, uv ).xyz * 2.0 - 1.0;
	prepareMaterial(material);
	material.baseColor = texture( albedoMap, uv );
	material.baseColor.rgb = sRGBToLinear(pow(material.baseColor.rgb,vec3(gamma)));
	if (bTransparent == 1)
		material.baseColor.rgb *= material.baseColor.a;
	int originalTransparent = 0;
	if (material.baseColor.a < 0.01f)
		originalTransparent = 1;
	vec4 mr = texture(roughness_metallicMap, uv);
	material.roughness = mr.g;
	material.metallic = mr.b;
	material.specularMask = mr.a;
	material.ambientOcclusion = texture(aoMap, uv).r;
	material.emissive.rgb = texture( EmissMap, uv ).rgb;
}

void getCommonPixelParams(const MaterialInputs material, inout PixelParams pixel)
{
	vec4 baseColor = material.baseColor;
	if (bTransparent == 1)
		unpremultiply(baseColor);
	pixel.diffuseColor = computeDiffuseColor(baseColor, material.metallic);
	float reflectance = computeDielectricF0(material.reflectance);
	pixel.f0 = computeF0( baseColor, material.metallic, reflectance );
}

void getRoughnessPixelParams(const MaterialInputs material, inout PixelParams pixel)
{
	float perceptualRoughness = material.roughness;
	pixel.perceptualRoughnessUnclamped = perceptualRoughness;
	perceptualRoughness = normalFiltering( perceptualRoughness, shading_geometricNormal );

	// Clamp the roughness to a minimum value to avoid divisions by 0 during lighting
    pixel.perceptualRoughness = clamp(perceptualRoughness, MIN_PERCEPTUAL_ROUGHNESS, 1.0);
    // Remaps the roughness to a perceptually linear roughness (roughness^2)
    pixel.roughness = perceptualRoughnessToRoughness(pixel.perceptualRoughness);
}

void getEnergyCompensationPixelParams(inout PixelParams pixel)
{
	pixel.dfg = prefilteredDFG(pixel.perceptualRoughness, shading_NoV);  //TODO
	pixel.energyCompensation = 1.0 + pixel.f0 * (1.0 / pixel.dfg.y - 1.0);
	pixel.energyCompensation = vec3(1.0f);
}

void getPixelParams(const MaterialInputs material, out PixelParams pixel) {
    getCommonPixelParams(material, pixel);
    getRoughnessPixelParams(material, pixel);
    getEnergyCompensationPixelParams(pixel);
}

void evaluateIBL(const MaterialInputs material, const PixelParams pixel, inout vec3 color)
{
	float ssao = 1.0f; //没有ssao
	float diffuseAO = min(material.ambientOcclusion, ssao);

	//specular layer
	vec3 Fr;
	vec3 E = specularDFG(pixel);
	vec3 r = getReflectedVector(pixel, shading_normal);
	Fr = E * prefilteredRadiance(r, pixel.perceptualRoughness);

	Fr *= pixel.energyCompensation;


	vec3 iblDiffuse = pixel.diffuseColor;
	if (bUseKajiya == 1)
		iblDiffuse = ApplyKajiya(material, pixel);

	vec3 diffuseNormal = shading_normal;

	vec3 diffuseIrradiance = diffuseIrradiance(diffuseNormal);
	vec3 Fd = iblDiffuse * diffuseIrradiance * (1.0 - E);

	color.rgb += (Fd + Fr )*ambientStrength;
	color.rgb *= (diffuseAO+AoOffset);
}

vec3 specularLobe( const PixelParams pixel, const Light light, const vec3 h,
				float NoV, float NoL, float NoH, float LoH)
{
	float D = distribution(pixel.roughness, NoH, h);
	float V = visibility(pixel.roughness, NoV, NoL);
	vec3 F = fresnel(pixel.f0, LoH);

	return (D * V) * F;
}

float diffuse(float roughness, float NoV, float NoL, float LoH)
{
	return 1.0 / PI;
}

vec3 diffuseLobe(const PixelParams pixel, float NoV, float NoL, float LoH)
{
	return pixel.diffuseColor * diffuse(pixel.roughness, NoV, NoL, LoH);
}

vec3 surfaceShading(const PixelParams pixel, const Light light, float occlusion)
{
	vec3 h = normalize(shading_view + light.l);

	float NoV = shading_NoV;
	float NoL = clamp( light.NoL, 0.0,1.0 );
	float NoH = clamp( dot(shading_normal, h), 0.0, 1.0 );
	float LoH = clamp( dot(light.l, h), 0.0, 1.0 );

	vec3 Fr = specularLobe(pixel, light, h, NoV, NoL, NoH, LoH);
	vec3 Fd = diffuseLobe(pixel, NoV, NoL, LoH);

	vec3 color = Fd + (Fr * pixel.energyCompensation);

	color = (color * light.colorIntensity.rgb) *
            (light.colorIntensity.w * light.attenuation * NoL);
	// change shaddow color and intensity
	if (shadowsEnable)
		color = ChangeShadowColor(occlusion, color);

	return color;
}

Light getDirectionalLight()
{
	Light light;
	light.colorIntensity = vec4( lightColors[0].xyz, lightDir[0].w);
	light.l = -lightDir[0].xyz;
	light.attenuation = 1.0f;
	light.NoL = clamp( dot( shading_normal, light.l ), 0.0, 1.0 );
	return light;
}

void evaluateDirectionalLight(const MaterialInputs material,
        const PixelParams pixel, inout vec3 color)
{
	Light light = getDirectionalLight();
	float visibility = 1.0f;
	if (shadowsEnable)
		visibility = clamp(ComputeShadow(FragPosLightSpace),0.0,1.0);

	color.rgb += surfaceShading(pixel, light, visibility);
}

vec4 evaluateLights(const MaterialInputs material)
{
	PixelParams pixel;
	getPixelParams( material, pixel ); 

	vec3 color = vec3(0.0);

	evaluateIBL(material, pixel, color);

	evaluateDirectionalLight(material, pixel, color);

	if (bTransparent == 1)
		color *= material.baseColor.a;

	return vec4( color, computeDiffuseAlpha(material.baseColor.a) );
}

void addEmissive( const MaterialInputs material, inout vec4 color )
{
	vec4 emissve = material.emissive;
	color.rgb += (emissve.rgb * color.a);
}

vec3 SkinDiffuse(float curv, vec3 NdotL)
{
	vec3 lookup = saturateMediump( NdotL * 0.5 + 0.65 );
	vec3 diffuse;
	diffuse.r = texture(PreintegratedSkinLut, vec2(lookup.r, 1.0 - curv)).r;
	diffuse.g = texture(PreintegratedSkinLut, vec2(lookup.g, 1.0 - curv)).g;
	diffuse.b = texture(PreintegratedSkinLut, vec2(lookup.b, 1.0 - curv)).b;
	return diffuse;
}

vec4 evaluateSkin(const MaterialInputs material )
{
	PixelParams pixel;
	getPixelParams( material, pixel );

	vec3 L = normalize(-lightDir[0].xyz);
    vec3 H = normalize(shading_view + L);
	float deltaWorldNormal = length(fwidth(Normal));
	float deltaWorldPos = length(fwidth(WorldPos));
	float curvatura = clamp((deltaWorldNormal / deltaWorldPos) * SkinCurveFactor,0.0,1.0);
	vec3 NdotL = vec3( dot(Normal, L) );
	vec3 diffuse = SkinDiffuse(curvatura, NdotL) * pixel.diffuseColor * SubsurfaceColor * lightDir[0].w * 0.5;

	float NoH = dot(shading_normal, H);
	float PH = pow(2.0 * texture(SkinSpecularBRDF, vec2(NoH, 1.0 - SkinSmooth * material.roughness)).g, 10.0);
	float LoH = clamp( dot(L, H), 0.0, 1.0 );
	vec3 F = fresnel(pixel.f0, LoH);
	vec3 specular = max(PH * F / dot(H, H), 0.0) * SkinSpecularScale;

	return vec4(vec3(ambientStrength * diffuse + specular), 1.0);
}

vec4 evaluateMaterial(const MaterialInputs material)
{
	vec4 color = vec4(0.0,0.0,0.0,1.0);
	if (EnableSkinRender)
	{
		color = evaluateSkin(material);
	}
	else
	{
		color = evaluateLights(material);
		addEmissive(material, color);
	}
	return color;
}

vec3 getBloomColor(vec3 color,  const MaterialInputs material)
{
	const highp float OneOverPreExposure = 0.98847;
	const highp float PreExposure = 1.01166;
	//const float BloomThreshold = 2.78598;
	highp vec3 sceneColor = color * OneOverPreExposure;
	sceneColor = min(vec3(256.0), sceneColor);
	sceneColor = max(vec3(0.0), sceneColor);

	highp vec3 LinearColor = sceneColor;
	highp float TotalLuminance = Luminance(LinearColor) * 1.01166;
	highp float  BloomLuminance = TotalLuminance - BloomThreshold;
	highp float  BloomAmount = clamp(BloomLuminance * 0.5f, 0.0, 1.0);

	sceneColor = BloomAmount * LinearColor * PreExposure;
	
	vec3 emiss = material.emissive.rgb;
	emiss = emiss * EmissStength * 10.0;
	if (length(emiss) > 0.01)
	{
		sceneColor = vec3(0.0);
	}
	//vec3 out_Emiss = vec3( sceneColor * BloomStrength * 0.1 + emiss );

	vec3 out_Emiss = vec3( material.emissive.rgb * BloomStrength * 10.0 );
	return out_Emiss;
}

void main()
{
	if (EnableRenderOutLine == 1)
	{
		float albedoMask = texture(OutLineMask, TexCoords).r;
		if (albedoMask < 0.5f)
			discard;
		out_FragColor = OutlineColor;
		out_EmissColor = vec4(0,0,0,1);
		return;
	}

	computeShadingParams();
	MaterialInputs inputs;
	initMaterial(inputs);

	material(inputs);

	vec4 out_color = evaluateMaterial(inputs);
	if (u_EnbleEmiss > 0)
	{
		out_FragColor = vec4(out_color.rgb, out_color.a);
		out_EmissColor = vec4( getBloomColor(out_color.rgb, inputs), 1.0 );
	}
	else
	{
		out_FragColor = vec4( ToneMapping(out_color.rgb), out_color.a );
		out_EmissColor = vec4(0,0,0,1);
	}
}


////shadow map calculation
//float ShadowCalculation(vec4 fragPosLightSpace)
//{
//    // perform perspective divide
//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    // Transform to [0,1] range
//    projCoords = projCoords * 0.5 + 0.5;
//    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
//    float closestDepth = texture(shadowMap, projCoords.xy).r; 
//    // Get depth of current fragment from light's perspective
//    float currentDepth = projCoords.z;
//    // Calculate bias (based on depth map resolution and slope)
//    vec3 normal = normalize(Normal);
//   // vec3 shadowlightDir = normalize(lightPos - WorldPos);
//	float dotValue = abs(dot(normal, -lightDir[0]));
//    float bias = max(0.01 * (1.0 - dotValue), 0.005);
//
//	if (dotValue < 0.15)
//		return 1.0;
//
//    // Check whether current frag pos is in shadow
//    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
//    // PCF
//    float shadow = 0.0;
//    vec2 texelSize = vec2(1.0) / vec2(textureSize(shadowMap, 0));
////	ivec2 texSize2d = textureSize(shadowMap, 0);
////	vec2 texelSize = vec2( 1.0 / float(texSize2d.x), 1.0 / float(texSize2d.y) );
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
//          // shadow += pcfDepth;//currentDepth - bias > pcfDepth  ? 1.0 : 0.0;     
//            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;     
//        }    
//    }
//    shadow /= 9.0;
//
//
//
//	//shadow = (currentDepth + bias) > closestDepth  ? 1.0 : 0.0;
//   
//    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
//    if(projCoords.z > 1.0)
//        shadow = 0.0;
//        
//    return shadow;
//}
//
////------------------------------------------------------------------------------
//// Shadow sampling methods
////------------------------------------------------------------------------------
//
//vec2 computeReceiverPlaneDepthBias(const vec3 position) {
//    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
//    vec2 bias;
////#if SHADOW_RECEIVER_PLANE_DEPTH_BIAS == SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
//    vec3 du = dFdx(position);
//    vec3 dv = dFdy(position);
//
//    // Chain rule we use:
//    //     | du.x   du.y |^-T      |  dv.y  -du.y |T    |  dv.y  -dv.x |
//    // D * | dv.x   dv.y |     =   | -dv.x   du.x |  =  | -du.y   du.x |
//
//    bias = inverse(mat2(du.xy, dv.xy)) * vec2(du.z, dv.z);
////#else
////    bias = vec2(0.0);
////#endif
//    return bias;
//}
//
//float samplingBias(float depth, const vec2 rpdb, const vec2 texelSize)
//{
//	float samplingError = min( 2.0 * dot( texelSize, abs(rpdb)), 0.01 );
//	depth += samplingError;
//
//	return depth;
//}
//
//
//float Linstep(float a, float b, float v)
//{
//	return clamp((v - a) / (b - a),0.0,0.8);
//}
//
//// Reduces VSM light bleedning
//float ReduceLightBleeding(float pMax, float amount)
//{
//	// Remove the [0, amount] tail and linearly rescale (amount, 1].
//	return Linstep(amount, 1.0f, pMax);
//}
//
//float ChebyshevUpperBound(vec2 Moments, float t) 
//{
//	float Variance = Moments.y - Moments.x * Moments.x;
//	float MinVariance = 0.0000001;
//	Variance = max(Variance, MinVariance);
//
//	// Compute probabilistic upper bound.
//	float d = t - Moments.x;
//	float pMax = Variance / (Variance + d * d);
//
//	float lightBleedingReduction = 0.5;
//	pMax = ReduceLightBleeding(pMax, lightBleedingReduction);
//    
//    pMax /=0.8;
//     vec3 normal = normalize(Normal);
//    float dotValue = abs(dot(normal, -lightDir[0]));
//    float bias = max(0.01 * (1.0 - dotValue), 0.005);
//	return (t-bias <= Moments.x ? 1.0 : pMax);
//}
//
//float ComputeShadow(vec4 ShadowCoord)
//{
//	vec3 position = ShadowCoord.xyz / ShadowCoord.w;
//    position = position * 0.5 + 0.5;
//
//	vec3 Moments = texture(shadowMap, position.xy).xyz;
//    float shadow = ChebyshevUpperBound(Moments.xy, clamp(position.z,0.0,1.0));
//
//	return 1.0-(1.0-shadow)*Moments.z;
//}
//
//vec3 getNormalFromMap()
//{
//    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
//    vec3 Q1  = normalize(dFdx(WorldPos));
//    vec3 Q2  = normalize(dFdy(WorldPos));
//    vec2 st1 = dFdx(TexCoords);
//    vec2 st2 = dFdy(TexCoords);
//
//    vec3 N   = normalize(Normal);
//    vec3 T  = Q1*st2.t - Q2*st1.t;
//    if(length(T) <0.00001)
//    {
//        return Normal;
//    }
//    T = normalize(T);
//    vec3 B  = -normalize(cross(N, T));
//    mat3 TBN = mat3(T, B, N);
//    vec3 finalNormal = normalize(TBN * tangentNormal);
//    return finalNormal;
//}
//
//vec3 getNormalFromMapEXT()
//{
//	vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
//	vec3 posOffseted = WorldPos;
//    posOffseted.y += 1.;
//    float D = -dot(Normal, WorldPos);
//    float distToPlane = dot(Normal, posOffseted) + D;
//    vec3 proj = posOffseted - Normal * distToPlane;
//    vec3 T = normalize(proj - WorldPos);
//	vec3 N = normalize(Normal);
//	vec3 B  = -normalize(cross(N, T));
//    mat3 TBN = mat3(T, B, N);
//    vec3 finalNormal = normalize(TBN * tangentNormal);
//    return finalNormal;
//}
//
//// google filament扒下来的
//// 高光项
//float D_GGX(float NoH, float a) {
//    float a2 = a * a;
//    float f = (NoH * a2 - NoH) * NoH + 1.0;
//    return a2 / (PI * f * f);
//}
//
//vec3 F_Schlick(float VoH, vec3 f0) {
//    return f0 + (vec3(1.0) - f0) * pow(1.0 - VoH, 5.0);
//}
//
//float F_Schlick(float f0, float f90, float VoH) {
//    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
//}
//
//vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
//{
//    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
//}  
//
//float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
//    float a2 = a * a;
//    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
//    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
//    return 0.5 / (GGXV + GGXL);
//}
//
//float V_SmithGGXCorrelated_Fast(float NoV, float NoL, float a) {
//    // Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
//    float v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, a);
//    return v;
//}
//// clearCoat的V项 简化计算 其余两项还是使用之前的
//float V_Kelemen(float LoH) {
//    return 0.25 / (LoH * LoH);
//}
//
//// 漫反项
//float Fd_Lambert() {
//    return 1.0 / PI;
//}
//
////各项异性高光
//
//mat3 getTBN()
//{
////	vec3 posOffseted = WorldPos;
////    posOffseted.y += 1.;
////    float D = -dot(Normal, WorldPos);
////    float distToPlane = dot(Normal, posOffseted) + D;
////    vec3 proj = posOffseted - Normal * distToPlane;
////    vec3 T = normalize(proj - WorldPos);
////	vec3 N = normalize(Normal);
////	vec3 B  = -normalize(cross(N, T));
////    mat3 TBN = mat3(T, B, N);
////
////	return TBN;
//
//	vec3 n = normalize( Normal);
//
//	vec3 t = normalize( Tangent.xyz);
//	vec3 b = normalize( cross(n, t) * sign(Tangent.w));
//	mat3 tbn = mat3(t, b, n);
//	return tbn;
//}
//
//float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) 
//{
//    // Burley 2012, "Physically-Based Shading at Disney"
//
//    // The values at and ab are perceptualRoughness^2, a2 is therefore perceptualRoughness^4
//    // The dot product below computes perceptualRoughness^8. We cannot fit in fp16 without clamping
//    // the roughness to too high values so we perform the dot product and the division in fp32
//    float a2 = at * ab;
//    highp vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
//    highp float d2 = dot(d, d);
//    float b2 = a2 / d2;
//    return a2 * b2 * b2 * (1.0 / PI);
//}
//
//float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
//        float ToL, float BoL, float NoV, float NoL) {
//    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
//    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
//    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
//    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
//    float v = 0.5 / (lambdaV + lambdaL);
//    return saturateMediump(v);
//}
//
//vec3 anisotropicSpecular( vec3 n, vec3 v, vec3 l, vec3 h, float NoV, float NoL, float NoH, float LoH, float a, vec3 f0 )
//{
//	vec3 t = normalize( getTBN() * vec3(1.0, 0.0, 0.0));
//    vec3 b = -normalize(cross(n, t));
//
//    float ToV = dot(t, v);
//    float BoV = dot(b, v);
//    float ToL = dot(t, l);
//    float BoL = dot(b, l);
//    float ToH = dot(t, h);
//    float BoH = dot(b, h);
//
//    float at = max( a * (1.0 + anisotropy), MIN_ROUGHNESS );
//    float ab = max( a * (1.0 - anisotropy), MIN_ROUGHNESS );
//
//    float D = D_GGX_Anisotropic( at, ab, ToH, BoH, NoH );
//    float V = V_SmithGGXCorrelated_Anisotropic(at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
//    vec3  F = F_Schlick(LoH, f0);
//
//    return (D * V) * F;
//}
//
//
//vec3 BRDF(vec3 n, vec3 v, vec3 l, float NoL,float roughness, float metalic, vec3 diffuseColor,vec3 radiance) {
//    vec3 h = normalize(v + l);
//
//    float NoV = abs(dot(n, v)) + 1e-5;
//    float NoH = clamp(dot(n, h), 0.0, 1.0);
//    float LoH = clamp(dot(l, h), 0.0, 1.0);
//    // perceptually linear roughness (see parameterization)
//    // remapping
//    float a = roughness * roughness;
//    float D = D_GGX(NoH, a);
//
//    float alpha = clamp(D,0.0,1.0);
//    g_alpha = alpha *(1.0-g_alpha);
//
//    vec3 diffuse = diffuseColor *(1.0-g_alpha) + radiance * g_alpha;
//
//    float reflectance = 0.5;
//    vec3  g_f0 = 0.16 * reflectance * reflectance * (1.0 - metalic) + diffuse * metalic;
//    vec3  F = F_Schlick(LoH, g_f0);
//    float V = V_SmithGGXCorrelated_Fast(NoV, NoL, a);
//    vec3 Fr = (D * V) * F;
//    //vec3 Fr = anisotropicSpecular( n, v, l, h, NoV, NoL, NoH, LoH, a, g_f0 );
//
//    vec3 kD = vec3(1.0) - F;
//    kD *= 1.0 - metalic;   
//    vec3 Fd = kD * diffuse * Fd_Lambert();
//    vec3 brdfLobe = Fd + Fr;
//    return brdfLobe;
//}
//
//vec3 ACESFilm(vec3 color)
//{
//	vec3 x = 0.8 * color;
//	float a = 2.51f;
//	float b = 0.03f;
//	float c = 2.43f;
//	float d = 0.59f;
//	float e = 0.14f;
//	return clamp((x*(a*x + b)) / (x*(c*x + d) + e),0.0,1.0);
//}
// 
//float LinearToSrgbChannel(float lin)
//{
//	if (lin < 0.00313067) return lin * 12.92;
//	return pow(lin, (1.0 / 2.4)) * 1.055 - 0.055;
//}
//
//vec3 LinearToSrgb(vec3 lin)
//{
//	//return pow(lin, 1/2.2);
//	return vec3(LinearToSrgbChannel(lin.r), LinearToSrgbChannel(lin.g), LinearToSrgbChannel(lin.b));
//}
//
//float SrgbToLinearChannel(float lin)
//{
//	if (lin < 0.04045) return lin/ 12.92;
//	return pow(lin / 1.055 + 0.0521327,2.4);
//}
//
//
//vec3 sRGBToLinear(vec3 Color)
//{
//	Color = max(vec3(6.10352e-5), Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
//    return vec3(SrgbToLinearChannel(Color.r), SrgbToLinearChannel(Color.g), SrgbToLinearChannel(Color.b));
//}
//
//vec3 ToneMapping(vec3 Color)
//{
//	return LinearToSrgb(ACESFilm(Color));
//}
//
//float Luminance(  vec3  LinearColor )
//{
//	return dot( LinearColor,  vec3 ( 0.3, 0.59, 0.11 ) );
//}
//
//
////Function main 
//void main()
//{  
//
//    vec4 srcColor = texture(albedoMap, TexCoords);
//    if(materialType == 1)
//    {
//        out_FragColor = srcColor;
//        return;
//    }
//
//    g_alpha = srcColor.a;
//    vec3 albedo     = sRGBToLinear(pow(srcColor.rgb, vec3(u_gamma)));// ;
//    
//    //albedo += emiss;
//    vec4 mr = texture(roughness_metallicMap, TexCoords);
//    if(u_EnbleRMGamma >0)
//    {
//        mr.g = pow(mr.g, 1.0/2.2);
//    }
//    
//    // out_FragColor = vec4(normalize(Normal)*0.5+vec3(0.5),1.0);
//    // out_EmissColor = vec4(emiss, 1.0);
//    // return;
//    float metallic  = mr.b;
//    float roughness = mr.g;
//    float ao        = texture(aoMap, TexCoords).r;
//    vec3 N = getNormalFromMapEXT();
//    vec3 V = normalize(camPos - WorldPos);
//
//    vec3 F0 = vec3(0.04); 
//   
//    F0 = mix(F0, albedo, metallic);
//
//	// reflectance equation
//    vec3 Lo = vec3(0.0);
//    for(int i = 0; i < LightNum; ++i) 
//    {
//        // calculate per-light radiance
//        vec3 L = normalize(-lightDir[i]);
//        vec3 H = normalize(V + L);
//        vec3 radiance = lightColors[i] ;
//
//        float NdotL = max(dot(N, L), 0.0);      
//        //float NdotL = dot(N, L)*0.5+0.5;      
//
//        vec3 BRDFterm = BRDF(N,V,L, NdotL, roughness, metallic, albedo,radiance);  
//
//        Lo += BRDFterm* radiance * NdotL *lightStrength[i];  
//    }   
//        // ambient lighting (we now use IBL as the ambient term)
//    vec3 R = reflect(-V, N);
//    R = (RotateIBL*vec4(R,1.0)).xyz;
//    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
//    float kD =  1.0 - metallic;	   
//  //  N.y = -N.y;
//    R.y = -R.y;
//    vec3 irradiance = texture(irradianceMap, N).rgb;
//    vec3 diffuse      = irradiance* albedo;
//    const float MAX_REFLECTION_LOD =7.0;
//    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
//    vec3 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rgb;
//    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
//    //vec3 ambient = ambientStrength*(kD * diffuse + specular);
//
//	//calculate shadow
//	
//    //float shadowCoef = shadowsEnable ? ShadowCalculation(FragPosLightSpace) : 0.0;
//	//float shadowCoef = shadowsEnable ? ShadowSample_PCF_Low(FragPosLightSpace) : 0.0;
//	//shadowCoef = min(shadowCoef,0.75);
//
//	vec3 ambient = ambientStrength * ((kD) * diffuse + specular);
//    
//	vec3 color = ambient;
//    if(shadowsEnable)
//    {
//        float shadowCoef = clamp(ComputeShadow(FragPosLightSpace),0.0,1.0);
//        color += Lo * (shadowCoef);
//    }
//    else{
//        color += Lo;
//    }
//
//	
//    const highp float OneOverPreExposure = 0.98847;
//	const highp float PreExposure = 1.01166;
//	//const float BloomThreshold = 2.78598;
//	highp vec3 sceneColor = color * OneOverPreExposure;
//	sceneColor = min(vec3(256.0), sceneColor);
//
//	sceneColor = max(vec3(0.0), sceneColor);
//
//	highp vec3 LinearColor = sceneColor;
//	highp float TotalLuminance = Luminance(LinearColor) * 1.01166;
//	highp float  BloomLuminance = TotalLuminance - BloomThresold;
//	highp float  BloomAmount = clamp(BloomLuminance * 0.5f, 0.0, 1.0);
//
//	float emiss_alpha = 1.0;
//	// if (BloomAmount > 0.0)
//	// {
//	// 	emiss_alpha = 0.0;
//	// }
//
//	sceneColor = BloomAmount * LinearColor * PreExposure;
//    color = ao*color;
//	//color = pow(color, vec3(1.0/2.2)); 
//
//	vec3 bloom_color = sceneColor;
//
//	//out_FragColor = vec4(color, srcColor.a);
//	if(u_EnbleEmiss>0)
//    {
//		out_FragColor = vec4(color, srcColor.a);
//		
//        vec3 emiss = sRGBToLinear(texture(EmissMap, TexCoords).rgb);
//        vec3 emiss_color = emiss  * EmissStength * 10.0;
//        if ( length(emiss_color) > 0.01 )
//	    {
//            emiss_alpha = 1.0;
//            bloom_color = vec3(0.0);
//        }
//        out_EmissColor = vec4(bloom_color * BloomStrength * 0.1 + emiss_color, emiss_alpha);
//    }
//    else
//    {
//        out_FragColor = vec4(ToneMapping(color),srcColor.a);
//        out_EmissColor = vec4(0.0,0.0,0.0,1.0);
//    }
//}
//
