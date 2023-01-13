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

#define float2   vec2
#define half	 float

in vec3 Normal;
in vec3 WorldPos;
in vec2 uv_MainTex;
in vec4 FragPosLightSpace;

layout (location = 0) out vec4 out_FragColor;

uniform sampler2D _IrisColorTex;
uniform sampler2D _IrisTex;
uniform sampler2D _MainTex;
uniform sampler2D _ShadeScleraTex;
uniform sampler2D _ShadeIrisTex;
uniform sampler2D _CorneaBump;
uniform sampler2D _EyeBump;
uniform sampler2D _IrisBump;

uniform float _scleraShadowAmt;
uniform float _irisShadowAmt;
//uniform vec4 _albedoColor;
//uniform vec4 reflectionMatte;
//uniform vec4 irradianceTex;
//uniform vec3 albedoColor;
//uniform float _roughness;
//uniform float _reflective;
//uniform float _metalMap;
//uniform float _ambientMap;
uniform float _irisSize;
uniform float _scleraSize;
float _pupilSize;
uniform float _limbus;

uniform vec4 _scleraColor;
uniform vec4 _irisColor;
//uniform vec4 _irisColorB;
//uniform vec4 _pupilColor;
uniform vec4 _illumColor;
float _parallax;
//uniform float _brightShift;
float irismasktex;
//uniform float irisoffsettex;
uniform float _smoothness;
uniform float _specsize;
uniform float _reflectTerm;

uniform vec3 camPos;
uniform vec3 lightDir[4];

vec3 UnpackNormal(vec3 rgb)
{
	return rgb * 2.0 -1.0;
}

float2 ParallaxOffset( float h, float height, vec3 veiwDir )
{
	h = h * height - height / 2.0;
	vec3 v = normalize(veiwDir);
	v.z += 0.42;
	return h * ( v.xy / v.z );
}

void main()
{
	vec3 viewDir = normalize(camPos - WorldPos);
	//calculate normal maps
	vec3 cBump = UnpackNormal(texture(_CorneaBump, vec2((uv_MainTex.x*_irisSize)-((_irisSize-1.0)/2.0),(uv_MainTex.y*_irisSize)-((_irisSize-1.0)/2.0))).rgb);

	float iSize2 = _irisSize*0.5;
	vec3 iBump = UnpackNormal(texture(_IrisBump, vec2((uv_MainTex.x*iSize2)-((iSize2-1.0)/2.0),(uv_MainTex.y*iSize2)-((iSize2-1.0)/2.0))).rgb);

	//CALCULATE ALBEDO MAP (SCLERA)
	vec4 scleratex = texture(_MainTex, vec2((uv_MainTex.x*_scleraSize)-((_scleraSize-1.0)/2.0),(uv_MainTex.y*_scleraSize)-((_scleraSize-1.0)/2.0)));
	scleratex.rgb = mix(scleratex.rgb,scleratex.rgb*_scleraColor.rgb,_scleraColor.a);
	vec3 eBump = UnpackNormal(texture(_EyeBump, vec2((uv_MainTex.x*_scleraSize)-((_scleraSize-1.0)/2.0),(uv_MainTex.y*_scleraSize)-((_scleraSize-1.0)/2.0))).rgb);

		//SCLERA SHADING
	float scleraShade = texture(_ShadeScleraTex, vec2((uv_MainTex.x*_scleraSize)-((_scleraSize-1.0)/2.0),(uv_MainTex.y*_scleraSize)-((_scleraSize-1.0)/2.0))).r;
	scleratex.rgb *= 1 - (scleraShade * _scleraShadowAmt);

	//CALCULATE ALBEDO MAP (IRIS)
	irismasktex = texture(_MainTex, float2((uv_MainTex.x*_irisSize)-((_irisSize-1.0)/2.0),(uv_MainTex.y*_irisSize)-((_irisSize-1.0)/2.0))).a;

	//FINAL NORMAL COMBINATION
	vec3 new_Normal = mix(eBump,cBump,irismasktex);

	// get mask texture
	float uvMask = 1.0 - texture(_IrisTex,uv_MainTex).b;

	//CALCULATE IRIS TEXTURE
	half iSize = _irisSize * 0.6;
	float2 irUVc = uv_MainTex;
	irUVc = float2((uv_MainTex.x*iSize)-((iSize-1.0)/2.0),((uv_MainTex.y)*iSize)-((iSize-1.0)/2.0));

	//TODO
	_pupilSize = 0.27;
	_pupilSize = mix(mix(0.5,0.2,iSize/5),mix(1.2,0.75,iSize/5),_pupilSize);
	irUVc = (irUVc*((-1.0+(uvMask*_pupilSize)))-(0.5*(uvMask*_pupilSize)));

	//CALCULATE PARALLAX COORDS
	float2 irUVp;
	irUVp = float2((uv_MainTex.x*iSize)-((iSize-1.0)/2.0),(uv_MainTex.y*iSize)-((iSize-1.0)/2.0));

	//TODO
	_parallax = 0.05;
	_parallax = _parallax * 2.0;
	half plxtex = texture(_IrisTex,irUVp).g;
	float _Parallax = mix(0.0,_parallax,plxtex);

	//CALCULATE IRIS/PUPIL MASK TEXTURES
	float2 irUV;
	irUV.x = mix((uv_MainTex.x*0.75)-((0.75-1.0)/2.0),(uv_MainTex.x*_pupilSize)-((_pupilSize-1.0)/2.0),uv_MainTex.x);
	irUV.y = mix((uv_MainTex.y*0.75)-((0.75-1.0)/2.0),(uv_MainTex.y*_pupilSize)-((_pupilSize-1.0)/2.0),uv_MainTex.y);

	vec3 vDir = viewDir;
	vDir.x = clamp(vDir.x,-0.75,0.75);
	vDir.z = clamp(vDir.z,-0.75,0.75);

	float2 offset = ParallaxOffset(texture(_IrisTex,irUV).b, _Parallax,vDir) * plxtex;
	offset.x = clamp(offset.x,-0.1,0.1);
	offset.y = clamp(offset.y,-0.1,0.1);

	//get iris and pupil texture
	vec4 irisColTex = texture(_IrisColorTex,irUVc-offset);


	//IRIS SHADING
	half irisShade = texture(_ShadeIrisTex, irUVc-offset).r;
	irisColTex.rgb *= 1 - (irisShade * _irisShadowAmt);

	half cNdotV2 = clamp(mix(-0.2,0.25,max(0,dot(eBump,viewDir))), 0.0, 1.0);
	float2 offset2 = ParallaxOffset(texture(_IrisTex,irUV).b, 0.5*cNdotV2,vDir);
	float irisoffsettex = texture(_IrisColorTex, irUVc-offset2).a;

	//combine sclera and iris colors
	irisColTex.rgb = mix(irisColTex.rgb,irisColTex.rgb*_irisColor.rgb,_irisColor.a);
	vec3 Albedo = mix(scleratex.rgb,irisColTex.rgb,irismasktex);

	//backscatter effects
	vec3 Emission = Albedo*(2.0*_illumColor.a)*_illumColor.rgb * irismasktex * (1-irisColTex.a);
	

	//----------------------------
	//##  CALCULATE SUBDERMAL  ##
	//----------------------------
	half lightFac = max(0,dot(lightDir[0].xyz,new_Normal));
	half h3 = max(0,dot(iBump,lightDir[0].xyz));
	half edge2 = clamp(mix(0.5,-0.5,max(0,dot(new_Normal,viewDir))), 0.0, 1.0);

	float alpha = 1.0;

	//-------------------------------
	//##  LIGHT TERM CALCULATION  ##
	//-------------------------------
	vec3 useNormal = cBump;
    half NdotV2 = max(0,dot(cBump,viewDir));
    half h = max(0,dot(useNormal,normalize(normalize(lightDir[0].xyz)+viewDir)));

	//---------------------------
	//##  INDEX OF REFRACTION  ##
	//---------------------------
	vec3 f0 = vec3(0,0,0);

	//------------------------------------
	//##  FRESNEL CALULATION (Schlick)  ##
	//------------------------------------
	vec3 fresnel;
	fresnel = f0+(1.0-f0)*pow((dot(cBump,normalize(lightDir[0].xyz + viewDir))),5);
	fresnel = fresnel * (f0+(1.0-f0)*pow((1.0-NdotV2),5));
	fresnel = clamp(max(fresnel,f0+(1.0-f0)*pow((1.0-NdotV2),5)), 0.0, 1.0);

	//add edge specular
	Albedo = (Albedo + (fresnel * 0.5 * h));

	//Limbus Darkening
	Albedo = mix(Albedo, Albedo * (0 - (_limbus * 20.0)), clamp(irisoffsettex * (2.0-irismasktex) * (irismasktex) * (cNdotV2*1), 0.0, 1.0));

	//-------------------------
	//##  UNITY 5 Features  ##
	//-------------------------
	float Specular =  mix(0.05, clamp(mix(_reflectTerm*1.1,_reflectTerm*1.2,irismasktex), 0.0, 1.0), irismasktex);
	Specular = mix(Specular * 0.025, Specular * 16, irismasktex);
	
	//o.Smoothness = lerp(0.8, 0.6, saturate(lerp(-2,5,irismasktex)) );
	float Smoothness = mix(_smoothness, _specsize, clamp(mix(-2,5,irismasktex), 0.0, 1.0) );

	//final brightness shift
	Albedo = mix(Albedo, Albedo * 2, irismasktex * 2);

	out_FragColor = vec4( Albedo, alpha );
}