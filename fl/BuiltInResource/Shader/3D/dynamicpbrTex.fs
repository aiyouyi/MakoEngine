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
in vec2 EmissUV;
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

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform mat4 RotateIBL;
// lights
uniform vec3 lightDir[4];
uniform vec3 lightColors[4];
uniform float lightStrength[4];
uniform int LightNum;
uniform float ambientStrength;
uniform float EmissStength;
uniform float u_gamma;
uniform int u_EnbleRMGamma;
//uniform vec3 EmissAddColor;
uniform float BloomThresold;
uniform float BloomStrength;

uniform float RotateSpeed;

uniform vec3 camPos;
uniform int materialType;
const float PI = 3.14159265359;
float g_alpha = 1.0;

#define MIN_ROUGHNESS            0.002025 
float anisotropy = 0.8;

#define FLT_EPS            1e-5
#define saturateMediump(x) x

//音乐壁纸
vec2 uv_center = vec2(0.5, 0.5);

// --------------------------------------------------------------------------

//shadow map calculation
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
   // vec3 shadowlightDir = normalize(lightPos - WorldPos);
	float dotValue = abs(dot(normal, -lightDir[0]));
    float bias = max(0.01 * (1.0 - dotValue), 0.005);

	if (dotValue < 0.15)
		return 1.0;

    // Check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = vec2(1.0) / vec2(textureSize(shadowMap, 0));
//	ivec2 texSize2d = textureSize(shadowMap, 0);
//	vec2 texelSize = vec2( 1.0 / float(texSize2d.x), 1.0 / float(texSize2d.y) );
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
          // shadow += pcfDepth;//currentDepth - bias > pcfDepth  ? 1.0 : 0.0;     
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;     
        }    
    }
    shadow /= 9.0;



	//shadow = (currentDepth + bias) > closestDepth  ? 1.0 : 0.0;
   
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

//------------------------------------------------------------------------------
// Shadow sampling methods
//------------------------------------------------------------------------------

vec2 computeReceiverPlaneDepthBias(const vec3 position) {
    // see: GDC '06: Shadow Mapping: GPU-based Tips and Techniques
    vec2 bias;
//#if SHADOW_RECEIVER_PLANE_DEPTH_BIAS == SHADOW_RECEIVER_PLANE_DEPTH_BIAS_ENABLED
    vec3 du = dFdx(position);
    vec3 dv = dFdy(position);

    // Chain rule we use:
    //     | du.x   du.y |^-T      |  dv.y  -du.y |T    |  dv.y  -dv.x |
    // D * | dv.x   dv.y |     =   | -dv.x   du.x |  =  | -du.y   du.x |

    bias = inverse(mat2(du.xy, dv.xy)) * vec2(du.z, dv.z);
//#else
//    bias = vec2(0.0);
//#endif
    return bias;
}

float samplingBias(float depth, const vec2 rpdb, const vec2 texelSize)
{
	float samplingError = min( 2.0 * dot( texelSize, abs(rpdb)), 0.01 );
	depth += samplingError;

	return depth;
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
    float dotValue = abs(dot(normal, -lightDir[0]));
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

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    vec3 Q1  = normalize(dFdx(WorldPos));
    vec3 Q2  = normalize(dFdy(WorldPos));
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = Q1*st2.t - Q2*st1.t;
    if(length(T) <0.00001)
    {
        return Normal;
    }
    T = normalize(T);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    vec3 finalNormal = normalize(TBN * tangentNormal);
    return finalNormal;
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

// google filament扒下来的
// 高光项
float D_GGX(float NoH, float a) {
    float a2 = a * a;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * f * f);
}

vec3 F_Schlick(float VoH, vec3 f0) {
    return f0 + (vec3(1.0) - f0) * pow(1.0 - VoH, 5.0);
}

float F_Schlick(float f0, float f90, float VoH) {
    return f0 + (f90 - f0) * pow(1.0 - VoH, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  

float V_SmithGGXCorrelated(float NoV, float NoL, float a) {
    float a2 = a * a;
    float GGXL = NoV * sqrt((-NoL * a2 + NoL) * NoL + a2);
    float GGXV = NoL * sqrt((-NoV * a2 + NoV) * NoV + a2);
    return 0.5 / (GGXV + GGXL);
}

float V_SmithGGXCorrelated_Fast(float NoV, float NoL, float a) {
    // Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
    float v = 0.5 / mix(2.0 * NoL * NoV, NoL + NoV, a);
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

//各项异性高光

mat3 getTBN()
{
//	vec3 posOffseted = WorldPos;
//    posOffseted.y += 1.;
//    float D = -dot(Normal, WorldPos);
//    float distToPlane = dot(Normal, posOffseted) + D;
//    vec3 proj = posOffseted - Normal * distToPlane;
//    vec3 T = normalize(proj - WorldPos);
//	vec3 N = normalize(Normal);
//	vec3 B  = -normalize(cross(N, T));
//    mat3 TBN = mat3(T, B, N);
//
//	return TBN;

	vec3 n = normalize( Normal);

	vec3 t = normalize( Tangent.xyz);
	vec3 b = normalize( cross(n, t) * sign(Tangent.w));
	mat3 tbn = mat3(t, b, n);
	return tbn;
}

float D_GGX_Anisotropic(float at, float ab, float ToH, float BoH, float NoH) 
{
    // Burley 2012, "Physically-Based Shading at Disney"

    // The values at and ab are perceptualRoughness^2, a2 is therefore perceptualRoughness^4
    // The dot product below computes perceptualRoughness^8. We cannot fit in fp16 without clamping
    // the roughness to too high values so we perform the dot product and the division in fp32
    float a2 = at * ab;
    highp vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    highp float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * (1.0 / PI);
}

float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
        float ToL, float BoL, float NoV, float NoL) {
    // Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
    // TODO: lambdaV can be pre-computed for all the lights, it should be moved out of this function
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    float v = 0.5 / (lambdaV + lambdaL);
    return saturateMediump(v);
}

vec3 anisotropicSpecular( vec3 n, vec3 v, vec3 l, vec3 h, float NoV, float NoL, float NoH, float LoH, float a, vec3 f0 )
{
	vec3 t = normalize( getTBN() * vec3(1.0, 0.0, 0.0));
    vec3 b = -normalize(cross(n, t));

    float ToV = dot(t, v);
    float BoV = dot(b, v);
    float ToL = dot(t, l);
    float BoL = dot(b, l);
    float ToH = dot(t, h);
    float BoH = dot(b, h);

    float at = max( a * (1.0 + anisotropy), MIN_ROUGHNESS );
    float ab = max( a * (1.0 - anisotropy), MIN_ROUGHNESS );

    float D = D_GGX_Anisotropic( at, ab, ToH, BoH, NoH );
    float V = V_SmithGGXCorrelated_Anisotropic(at, ab, ToV, BoV, ToL, BoL, NoV, NoL);
    vec3  F = F_Schlick(LoH, f0);

    return (D * V) * F;
}


vec3 BRDF(vec3 n, vec3 v, vec3 l, float NoL,float roughness, float metalic, vec3 diffuseColor,vec3 radiance) {
    vec3 h = normalize(v + l);

    float NoV = abs(dot(n, v)) + 1e-5;
    float NoH = clamp(dot(n, h), 0.0, 1.0);
    float LoH = clamp(dot(l, h), 0.0, 1.0);
    // perceptually linear roughness (see parameterization)
    // remapping
    float a = roughness * roughness;
    float D = D_GGX(NoH, a);

    float alpha = clamp(D,0.0,1.0);
    g_alpha = alpha *(1.0-g_alpha);

    vec3 diffuse = diffuseColor *(1.0-g_alpha) + radiance * g_alpha;

    float reflectance = 0.5;
    vec3  g_f0 = 0.16 * reflectance * reflectance * (1.0 - metalic) + diffuse * metalic;
    vec3  F = F_Schlick(LoH, g_f0);
    float V = V_SmithGGXCorrelated_Fast(NoV, NoL, a);
    vec3 Fr = (D * V) * F;
    //vec3 Fr = anisotropicSpecular( n, v, l, h, NoV, NoL, NoH, LoH, a, g_f0 );

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metalic;   
    vec3 Fd = kD * diffuse * Fd_Lambert();
    vec3 brdfLobe = Fd + Fr;
    return brdfLobe;
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

vec2 computeRotateUV(vec2 input_uv)
{
	vec2 uv = input_uv - uv_center;

	if (RotateSpeed == 0.0)
	{
		return input_uv;
	}

	float angle = radians( RotateSpeed );
	float sinAngle, cosAngle;
	sinAngle = sin(angle);
	cosAngle = cos(angle);

	mat2 rotationMatrix = mat2( cosAngle, -sinAngle, sinAngle, cosAngle );

	vec2 normalizedUV = normalize( rotationMatrix * uv );
	return normalizedUV;
}

//Function main 
void main()
{  

    vec4 srcColor = texture(albedoMap, TexCoords);
    if(materialType == 1)
    {
        out_FragColor = srcColor;
        return;
    }

	vec2 rotate_uv = computeRotateUV(EmissUV);

    vec3 emiss = texture(EmissMap, rotate_uv).rgb;
	//srcColor.a = max( emiss.x, max(emiss.y, emiss.z) );
	emiss = sRGBToLinear(emiss);

    g_alpha = srcColor.a;
    vec3 albedo     = sRGBToLinear(pow(srcColor.rgb, vec3(u_gamma)));// ;
    
    //albedo += emiss;
    vec4 mr = texture(roughness_metallicMap, TexCoords);
    if(u_EnbleRMGamma >0)
    {
        mr.g = pow(mr.g, 1.0/2.2);
    }
    
    // out_FragColor = vec4(normalize(Normal)*0.5+vec3(0.5),1.0);
    // out_EmissColor = vec4(emiss, 1.0);
    // return;
    float metallic  = mr.b;
    float roughness = max(mr.g, 0.01);
    float ao        = texture(aoMap, TexCoords).r;
    vec3 N = getNormalFromMapEXT();
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04); 
   
    F0 = mix(F0, albedo, metallic);

	// reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < LightNum; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(-lightDir[i]);
        vec3 H = normalize(V + L);
        vec3 radiance = lightColors[i] ;

        float NdotL = max(dot(N, L), 0.0);      
        //float NdotL = dot(N, L)*0.5+0.5;      

        vec3 BRDFterm = BRDF(N,V,L, NdotL, roughness, metallic, albedo,radiance);  

        Lo += BRDFterm* radiance * NdotL *lightStrength[i];  
    }   
        // ambient lighting (we now use IBL as the ambient term)
    vec3 R = reflect(-V, N);
    R = (RotateIBL*vec4(R,1.0)).xyz;
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float kD =  1.0 - metallic;	   
  //  N.y = -N.y;
    R.y = -R.y;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance* albedo;
    const float MAX_REFLECTION_LOD =7.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec3 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rgb;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    //vec3 ambient = ambientStrength*(kD * diffuse + specular);

	//calculate shadow
	
    //float shadowCoef = shadowsEnable ? ShadowCalculation(FragPosLightSpace) : 0.0;
	//float shadowCoef = shadowsEnable ? ShadowSample_PCF_Low(FragPosLightSpace) : 0.0;
	//shadowCoef = min(shadowCoef,0.75);

	vec3 ambient = ambientStrength * ((kD) * diffuse + specular);
    
	vec3 color = ambient;
    if(shadowsEnable)
    {
        float shadowCoef = clamp(ComputeShadow(FragPosLightSpace),0.0,1.0);
        color += Lo * (shadowCoef);
    }
    else{
        color += Lo;
    }

	const highp float OneOverPreExposure = 0.98847;
	const highp float PreExposure = 1.01166;
	//const float BloomThreshold = 2.78598;
	highp vec3 sceneColor = color * OneOverPreExposure;
	sceneColor = min(vec3(256.0), sceneColor);

	sceneColor = max(vec3(0.0), sceneColor);

	highp vec3 LinearColor = sceneColor;
	highp float TotalLuminance = Luminance(LinearColor) * 1.01166;
	highp float  BloomLuminance = TotalLuminance - BloomThresold;
	highp float  BloomAmount = clamp(BloomLuminance * 0.5f, 0.0, 1.0);

	float emiss_alpha = 1.0;
	// if (BloomAmount > 0.0)
	// {
	// 	emiss_alpha = 0.0;
	// }


	sceneColor = BloomAmount * LinearColor * PreExposure;

    color = ao*color;

	//color = pow(color, vec3(1.0/2.2)); 

	vec3 bloom_color = sceneColor;

	out_FragColor = vec4(color, srcColor.a);

	vec3 emiss_color = emiss  * EmissStength * 5.0;
	if ( length(emiss_color) > 0.01 )
	{
		emiss_alpha = 1.0;
		bloom_color = vec3(0.0);
	}
	
	out_EmissColor = vec4(bloom_color * BloomStrength * 0.1 + emiss_color, emiss_alpha);
}

