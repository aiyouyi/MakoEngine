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

#define FurBend 1
#define FurAllBend 1
#define AE_PointLightNum 0
#define FurColorTex 1
#define StrandSpec 1
#define AMAZING_USE_BONES 1
#define AE_DirLightNum 1
#define AE_SpotLightNum 0

uniform float _FURLEVEL;
precision highp float;
const float MAX_MIP_BIAS = 13.0;
const float PI = 3.1415926535897932384626433832795;

#ifdef FurColorTex
uniform sampler2D u_FurColorTex;
#endif

#ifdef EnvironmentTex
uniform sampler2D u_IrradianceTexture;
uniform float u_IBLRotation;
#endif 
uniform vec4 u_EnvironmentColor;
uniform float u_IBLExposure;
uniform sampler2D u_FBOTexture;

uniform vec4 u_FurColor;
uniform sampler2D u_FurStyleMask;

#ifdef FurLengthMask
uniform sampler2D u_FurLengthMask;
#endif 

uniform float u_FurMaskScale;
uniform float u_FurThickness;
uniform float u_FurThickness_B;
uniform float u_FurThickness_T;
uniform float u_FurThickness_B2T;
uniform float u_FurFeather;
uniform vec3 u_UVoffset;
#ifdef FlowMap
uniform sampler2D u_FlowMapTexture;
uniform float u_FlowMapScale;
uniform float u_FlowMapPow;
#endif

in vec3 varAO;
in float varRim;
in vec3 varDiffuse;
in vec3 varStrandSpecular;
in vec3 varWorldPosition;
in vec3 varWorldNormal;
in vec2 varTexcoord0;

layout (location = 0) out vec4 out_FragColor;
layout (location = 1) out vec4 out_EmissColor;

uniform vec3 camPos;
uniform bool drawSolid;

#ifdef SingleFurShadow
uniform vec4 u_ShaodowCol;
uniform float u_ShadowIntensity;
#endif

const int STANDARD_MODEL_ID     = 0;  // standard pbr

struct FSBuffer
{
    vec3        world_position;
    vec3        world_normal;
    vec3        world_tangent;
    vec3        world_binormal;
    mat3        TBN;
    vec3        world_view;
    vec2        texcoord0;
};
struct SurfaceProperties
{
    int         shading_model_id;
    vec3        baseColor;
    float       metallic;
    float       roughness;
    float       ao;
    vec3        normal;
    float       translucency;
    vec4        custom1;
    vec4        custom2;
    vec4        custom3;
    vec4        custom4;
};

void SurfacePropertiesToDefault(inout SurfaceProperties S)
{
    S.shading_model_id = STANDARD_MODEL_ID;
    S.baseColor      = vec3(1.0);
    S.metallic       = 0.5;
    S.roughness      = 0.5;
    S.ao             = 1.0;
    S.normal         = vec3(0.0);
    S.translucency   = 1.0;
    S.custom1        = vec4(1.0);
    S.custom2        = vec4(1.0);
    S.custom3        = vec4(1.0);
    S.custom4        = vec4(1.0);
}


struct ShadingBuffer
{
    int         shading_model_id;
    vec3        pos;
    vec3        n;
    vec3        t;
    vec3        bn;
    vec3        v;
    vec3        l;
    vec3        r;
    vec3        diffuse;
    vec3        specular;
    float       metallic;
    float       roughness;
    vec4        custom1;
    vec4        custom2;
    vec4        custom3;
    vec4        custom4;
};


// ibl sampler
vec3 DecodeRGBD(vec4 rgbd)
{
    float a = 1.0 / (rgbd.a);
    return rgbd.rgb * a;
}

float Atan2(float x, float y)
{
    float signx = x < 0.0 ? -1.0 : 1.0;
    return signx * acos(clamp(y / length(vec2(x, y)), -1.0, 1.0));
}

vec2 DirToPanoramicTexCoords(vec3 reflDir, float rotation)
{
    vec2 uv;
    uv.x = Atan2(reflDir.x, -reflDir.z) - PI / 2.0;
    uv.y = acos(reflDir.y);
    uv = uv / vec2(2.0 * PI, PI);

    uv.x += rotation;
    uv.x = fract(uv.x + floor(uv.x) + 1.0);

    return uv;
}


vec3 SampleIrradiance(sampler2D IrradianceTexture, vec3 N, float rotation)
{
    vec2 uv = DirToPanoramicTexCoords(N, rotation);
    vec4 irradiance = texture(IrradianceTexture, uv, -MAX_MIP_BIAS);
    return DecodeRGBD(irradiance);
}


// ibl shading

vec3 Irradiance(vec3 baseColor, vec3 irradiance)
{
    return baseColor * irradiance;
}



// algorithm
float GammaToLinear(float x)
{
    return pow(x, 2.2);
}

vec3 GammaToLinear(vec3 color)
{
    return vec3(GammaToLinear(color.r), GammaToLinear(color.g), GammaToLinear(color.b));
}

float LinearToGamma(float x)
{

    return pow(x, 1.0 / 2.2);
}

vec3 LinearToGamma(vec3 color)
{
    return vec3(LinearToGamma(color.r), LinearToGamma(color.g), LinearToGamma(color.b));
}

vec4 Texture2DScale(sampler2D sampler, vec2 uv, float scale)
{
    return texture(sampler, uv * scale);
}

vec3 SpecularColor(vec3 baseColor, float metallic)
{
    return mix(vec3(0.04), baseColor * metallic, metallic);
}

vec3 BaseColor(vec3 baseColor, float metallic)
{
    return mix(baseColor * (1.0 - metallic), vec3(0.0), metallic);
}

vec3 LinearToneMapping(vec3 x)
{
    float a = 1.8;  // Mid
    float b = 1.4;  // Toe
    float c = 0.5;  // Shoulder
    float d = 1.5;  // Mid
    return (x * (a * x + b)) / min(vec3(1000.0), (x * (a * x + c) + d));
}

vec3 TransformTangentToWorld(mat3 TBN, vec3 v)
{
    return normalize(TBN * v);
}

vec3 TransformTexTangentToWorld(mat3 TBN, vec3 v)
{
    return TransformTangentToWorld(TBN, v * 2.0 - vec3(1.0));
}

vec3 TransformWorldToTangent(vec3 v, mat3 TBN)
{
    return normalize(v * TBN);
}

vec3 RimColor(vec3 N, vec3 V, vec3 rimColor, float fallOff)
{
    float revNoV = 1.0 - max(0.0, dot(N,V));
    vec3 rc = rimColor * pow(revNoV, fallOff);
    return rc;
}

vec3 NormalBlendLinear(vec3 n1, vec3 n2)
{
    vec3 r = vec3(n1.rg + n2.rg, n1.b); // n1.b * n2.b
    return normalize(r);
}

vec2 RotateTexcoord(vec2 texcoord, vec2 pivot, float rotate)
{
    vec2 pivotOffset = texcoord  - pivot;
    float cosAngle = cos(rotate * 2.0 * PI);
    float sinAngle = sin(rotate * 2.0 * PI);
    mat2 rot = mat2(cosAngle, -sinAngle, sinAngle, cosAngle);
    return pivot + rot * pivotOffset;
}

float noise(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main ()
{
	if (drawSolid)
	{
		out_FragColor = texture( u_FurColorTex, varTexcoord0 );
		out_EmissColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
                    // Available parameters in FS.
                FSBuffer FB;
                FB.world_position      = varWorldPosition;
                FB.world_normal        = normalize(varWorldNormal);
                FB.world_view          = normalize(camPos - varWorldPosition);


                FB.texcoord0           = varTexcoord0;
               // FB.texcoord0.y         = 1.0 - FB.texcoord0.y;

                vec2 uvoffset = u_UVoffset.xy * _FURLEVEL * 0.1;

            #ifdef FurAllBend
            #ifdef FlowMap
                vec4 t_flowmap = Texture2DScale(u_FlowMapTexture, FB.texcoord0, u_FlowMapScale);
                vec2 t_uvoffset = t_flowmap.xy * 2.0 - vec2(1.0, 1.0);
                uvoffset = t_uvoffset * _FURLEVEL * u_FlowMapPow * 0.1 + uvoffset;
            #endif
            #endif

                // sampler
            #ifdef  FurColorTex
                vec4 t_albedo   = Texture2DScale(u_FurColorTex, FB.texcoord0 + uvoffset / u_FurMaskScale, 1.0);
            #endif
            //*********** User-fillable SurfaceProperties **********//
                SurfaceProperties SP;
                SurfacePropertiesToDefault(SP);
                SP.shading_model_id    = STANDARD_MODEL_ID;
            #ifdef FurColorTex
                //SP.baseColor           = GammaToLinear(t_albedo.rgb) * u_FurColor.rgb;
                SP.baseColor           = t_albedo.rgb * u_FurColor.rgb;
                SP.translucency        = t_albedo.a * u_FurColor.a;
            #else
                // SP.baseColor           = GammaToLinear(u_FurColor.rgb);
                SP.baseColor           = u_FurColor.rgb;
                SP.translucency        = u_FurColor.a;
            #endif
                float initTranslucency = SP.translucency;

        
                SP.metallic            = 0.0;
                SP.roughness           = 0.0;
                SP.ao                 = 1.0;
                SP.normal              = FB.world_normal;
                // SP.normal = normalize(SP.normal * dot(SP.normal, FB.world_view) - FB.world_view);
            //**************** EndSurfaceProperties ***************//

                // Data used in lighting calculations.
                ShadingBuffer SB;
                SB.shading_model_id    = SP.shading_model_id;
                SB.pos                 = FB.world_position;
                SB.n                   = SP.normal;

                SB.v                   = FB.world_view;
                //SB.r                   = reflect(-FB.world_view, SP.normal);
                SB.diffuse             = SP.baseColor * SP.ao;
                SB.specular            = SpecularColor(SP.baseColor, SP.metallic) * SP.ao;
                SB.metallic            = SP.metallic;
                SB.roughness           = SP.roughness;

                // initialization
                vec3 final_color = vec3(0.0);

                #ifdef FurLengthMask
                    vec4 t_lengthMask = Texture2DScale(u_FurLengthMask, FB.texcoord0, 1.0);
                    if(t_lengthMask.r < _FURLEVEL) discard;
                #endif 
                vec4 t_mask = Texture2DScale(u_FurStyleMask, FB.texcoord0 + uvoffset / u_FurMaskScale, u_FurMaskScale);

                float rBot = pow(t_mask.r + u_FurThickness , (10.0-u_FurThickness_B));
                float rTop = pow(t_mask.r + u_FurThickness , (10.0-u_FurThickness_T));
                float b2t = clamp(u_FurThickness_B2T,0.0,1.0);
                b2t = mix (-1.0 - b2t,b2t+0.1,step(0.0,b2t));
                t_mask.r = mix(rBot,rTop,mix(pow(_FURLEVEL*1.3333,-b2t),pow(_FURLEVEL*1.3333,b2t*10.0),step(0.0,b2t)));
                // t_mask.r = mix(rBot,rTop,pow(_FURLEVEL*1.3333,-u_FurThickness_B2T));
                SP.translucency = ((clamp(pow(1.1 - _FURLEVEL, 2.0),0.0,1.0) -1.0) *clamp(2.0 - u_FurThickness_B2T,0.0,1.0)  + t_mask.r) *pow((1.0 - _FURLEVEL * 1.3333),u_FurFeather);
                if (SP.translucency < 0.000001) discard;

                /*
                if(u_FurTailFactor < 0.0)
                {
                    SP.translucency = pow(1.1 - _FURLEVEL, 3.0 + u_FurTailFactor) - 1.0 + t_mask.r;
                    if (SP.translucency < 0.000001) discard;
                }
                else 
                {
                    SP.translucency = pow(t_mask.r, u_FurTailFactor) - pow(_FURLEVEL, u_FurTailFactor);
                    if (SP.translucency < 0.000001 && _FURLEVEL>0.0) discard;
                }
                */
                #ifdef SingleFurShadow
                vec2 ShadowDir =vec2(0.1);
                vec4 t_mask_lightOffset = Texture2DScale(u_FurStyleMask, FB.texcoord0 + (uvoffset+ShadowDir) / u_FurMaskScale, u_FurMaskScale);
                t_mask_lightOffset.r = pow(t_mask_lightOffset.r, 10.0-u_FurThickness_B);
                float singleFurDiffuse = step(0.0,t_mask.r - t_mask_lightOffset.r) ;
                #endif

                SP.translucency *= initTranslucency;

                #ifdef EnvironmentTex
                    vec3 irradiance = SampleIrradiance(u_IrradianceTexture, SB.n, u_IBLRotation); //Diffuse
                    vec3 iblDiffuse = Irradiance(SB.diffuse, irradiance); //Diffuse
                    final_color += iblDiffuse * varAO * u_IBLExposure;
                    final_color += iblDiffuse * varRim;
                #else
                    // final_color += vec3(0.18,0.12,0.08) * varAO * u_IBLExposure;
                    // final_color += vec3(0.18,0.12,0.08)* varRim;
                    final_color += u_EnvironmentColor.xyz*0.2 * varAO * u_IBLExposure;
                    final_color += u_EnvironmentColor.xyz*0.2 * varRim;
                #endif
                #ifdef SingleFurShadow
                    //final_color += mix(SB.diffuse * varDiffuse*u_ShaodowCol.xyz,SB.diffuse * varDiffuse,clamp(singleFurDiffuse + 1.0 -u_ShadowIntensity,0.0,1.0));
                    final_color += mix(SB.diffuse * varDiffuse*u_ShaodowCol.xyz* varAO,SB.diffuse * varDiffuse* varAO,clamp(singleFurDiffuse + 1.0 -u_ShadowIntensity,0.0,1.0));
                #else
                    //final_color += SB.diffuse * varDiffuse;
                      final_color += SB.diffuse * varDiffuse * varAO ;
                #endif
                   final_color +=   varStrandSpecular;
                // final_color = LinearToneMapping(final_color);
                // final_color = LinearToGamma(final_color);
                #ifdef MODEL_SURFACE
                    SP.translucency = 1.0;
                #endif

                out_FragColor = vec4(final_color, SP.translucency);
				out_EmissColor = vec4(0.0,0.0,0.0,1.0);

}
