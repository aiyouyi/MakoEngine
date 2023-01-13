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

in vec3 out_pos;
in vec2 T0;
in vec2 T1;
in vec3 out_normal;
in vec3 SH;
in vec4 Tangent;

layout (location = 0) out vec4 out_FragColor;
layout (location = 1) out vec4 out_EmissColor;

uniform sampler2D noiseMap;
uniform sampler2D colorMap;
//uniform sampler2D flowMap;
//uniform sampler2D lengthTexture;

uniform vec4 lightDir[4];
uniform vec4 lightColors[4];
uniform vec3 _OcclusionColor;
//uniform vec3 _Color;
uniform float LightFilter; //("平行光毛发穿透",  Range(-0.5,0.5)) = 0.0
uniform float FurLightExposure; //毛发曝光亮度， 默认为10
uniform float FresnelLV;
uniform float FurMask;
uniform float Tming;
uniform float _MainSpecShift;
uniform float _SecSpecShift;
uniform float _MainSpecSmooth;
uniform float _SecSpecSmooth;
uniform vec3 _MainSpecColor;
uniform vec3 _SecSpecColor;

uniform float ambientStrength;
uniform float FurAmbientStrength;
uniform float FurOffset;
uniform float AO;
uniform bool drawSolid;
uniform vec3 camPos;

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

vec3 getTangent()
{
	vec3 Q1  = normalize(dFdx(out_pos));
    vec3 Q2  = normalize(dFdy(out_pos));
    vec2 st1 = dFdx(T1);
    vec2 st2 = dFdy(T1);

    vec3 N   = normalize(out_normal);
    vec3 T  = Q1*st2.t - Q2*st1.t;
    if(length(T) <0.00001)
    {
        return out_normal;
    }
    T = normalize(T);

	vec3 B  = -normalize(cross(N, T));

	return B;
}

vec3 getTangent1()
{
	vec3 posOffseted = out_pos;
    posOffseted.y += 1.;
    float D = -dot(out_normal, out_pos);
    float distToPlane = dot(out_normal, posOffseted) + D;
    vec3 proj = posOffseted - out_normal * distToPlane;
    vec3 tangent = normalize(proj - out_pos);
    return tangent;
}

float StrandSpecular ( vec3 T, vec3 V, vec3 L, float exponent)             
{                 
	vec3 H = normalize ( L + V );                 
	float dotTH = dot ( T, H );                 
	float sinTH = sqrt( 1.0 - dotTH * dotTH);                 
	float dirAtten = smoothstep( -1.0, 0.0, dotTH );                 
	return dirAtten * pow(sinTH, exponent);             
} 

vec3 ShiftTangent ( vec3 T, vec3 N, float shift)             
{                 
	vec3 shiftedT = T + (shift * N);                 
	return normalize(shiftedT);             
} 

void main()
{
	if (drawSolid)
	{
		out_FragColor = texture(colorMap, T1);
		out_EmissColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 srcColor = texture( colorMap, T1 ).xyz;
	//vec3 baseColor = sRGBToLinear(srcColor);
	float Noise = texture( noiseMap, T0 ).r;
//	vec3 NoiseMask = texture(furTexture, T0).rgb;
//	vec3 RegionMask =  texture(lengthTexture, T1).rgb;
//	float Noise = NoiseMask.r * RegionMask.r + NoiseMask.g * RegionMask.g + NoiseMask.b * RegionMask.b;
//Ambient occlusion
	float Occlusion = FurOffset * FurOffset;
	Occlusion += 0.04;
	vec3 SHL = mix( lightColors[0].rgb * SH, vec3(SH), Occlusion );
	//vec3 SHL = mix( _OcclusionColor * SH, vec3(SH), Occlusion );

//次表面散射（太阳光）
	float NoL = dot(-lightDir[0].xyz, out_normal) * 0.5 + 0.5;
	float DirLight = clamp( NoL + LightFilter + FurOffset, 0.0, 1.0 );

//轮廓光
	vec3 V = normalize(camPos - out_pos);
	float Fresnel = 1.0 - max( 0.0, dot(out_normal, V) );
	vec3 RimLight = vec3( Fresnel * Occlusion );
	RimLight *= RimLight;
	RimLight *= FresnelLV * SH * srcColor;
	SHL += RimLight;

//各项异性高光
//	vec3 tangent = getTangent1();
//	vec3 jitter = texture(flowMap, T1).rgb;
//	vec3 Tmain = ShiftTangent( tangent, jitter, Noise + _MainSpecShift );
//	//vec3 Tmain = tangent + jitter;
//	vec3 Tsec = ShiftTangent (tangent, out_normal, Noise + _SecSpecShift ); 
//	vec3 specMain = vec3( StrandSpecular(Tmain, V, -lightDir[0], _MainSpecSmooth)*FurOffset * FurOffset); 
//	vec3 specSec = vec3( StrandSpecular(Tsec,V, -lightDir[0], _SecSpecSmooth)*FurOffset * FurOffset); 
//	vec3 specular = _MainSpecColor * specMain;
//	vec3 secspecular = _SecSpecColor * specSec; 


	vec3 color =  DirLight *srcColor*FurLightExposure * FurAmbientStrength +SHL*FurLightExposure;
	//vec3 color = specular + secspecular;
	out_FragColor.rgb = color;

	out_FragColor.a = clamp((Noise * 2.0 -(FurOffset * FurOffset +(FurOffset * FurMask * 5.0)))* Tming, 0.0, 1.0);
	out_EmissColor = vec4(0.0, 0.0, 0.0, 1.0);
//	Noise = step( FurOffset * FurOffset, Noise );
//	Noise *= 1.0 - FurOffset * FurOffset;
//	out_FragColor.a = Noise;
}