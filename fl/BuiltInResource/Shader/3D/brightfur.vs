#version 300 es
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 gBoneIDsArray;
layout(location = 5) in vec4 gBoneWeightsArray;

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
const int MAX_NUM_DIR = 3;
const int MAX_NUM_POINT = 2;
const int MAX_NUM_SPOT = 2;
const float PI = 3.1415926535897932384626433832795;
const vec2 Fur_Spec_Param = vec2(0.850000, 1.000000);
const vec4 Fur_Params = vec4(1.100000, 0.700000, 1.500000, 1.000000);

//attribute vec3 attPosition;
//attribute vec3 attNormal;
//attribute vec2 attTexcoord0;
//
//attribute vec3 attTangent;
//attribute vec3 attBinormal;

//#ifdef AMAZING_USE_BONES
//attribute vec4 attBoneIds;
//attribute vec4 attWeights;
//const int MAX_BONES = 50; // max
//uniform mat4 u_Palatte[MAX_BONES];
//#endif

//uniform vec3 u_WorldSpaceCameraPos;
//uniform mat4 u_MV;
//uniform mat4 u_Model;
//uniform mat4 u_View;
//uniform mat4 u_Projection;
//uniform mat4 u_TransposeInvModel;
uniform vec4 u_Time; //  x = gametime/20; y = gametime; z = gametime * 2; w = gametime * 3;
uniform bool AnimationEnable;
uniform mat4 meshMat;
const int MAX_BONES = 50;
uniform mat4 gBonesMatrix[MAX_BONES];
uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camPos;
uniform vec4 lightDir[4];
uniform vec4 lightColors[4];
uniform float lightStrength[4];
uniform float lightIntensity;
uniform vec3 vGravity;
uniform float vGravityStength;

out vec3 varWorldPosition;
out vec3 varWorldNormal;
out vec2 varTexcoord0;

out vec3 varAO;
out float varRim;
out vec3 varDiffuse;
out vec3 outDir;
//varying vec3 varFabricSpecular;
out vec3 varStrandSpecular;
uniform vec4 u_AOColor;
uniform float u_LightFilter;
#ifdef FurRim
uniform float u_RimIntensity;
#endif
// #ifdef FabricSpec
// uniform float u_SpecIntensity;
// #endif
#ifdef StrandSpec
uniform float u_Roughness;
uniform vec4 u_StrandSpecColor;
uniform float u_StrandExpHigh;
uniform float u_StrandExpLow;
uniform vec4 u_StrandSpecColor_low;
uniform vec2 u_StrandShift;
#endif

const int FUR_MODEL_ID = 1; // fur

//DirLight
#ifdef AE_DirLightNum
#if AE_DirLightNum
uniform float u_DirLightsEnabled[MAX_NUM_DIR];
uniform vec3 u_DirLightsDirection[MAX_NUM_DIR];
uniform vec3 u_DirLightsColor[MAX_NUM_DIR];
uniform float u_DirLightsIntensity[MAX_NUM_DIR];
#endif
#endif

//PointLight
#ifdef AE_PointLightNum
#if AE_PointLightNum
uniform float u_PointLightsEnabled[MAX_NUM_POINT];
uniform vec3 u_PointLightsPosition[MAX_NUM_POINT];
uniform vec3 u_PointLightsColor[MAX_NUM_POINT];
uniform float u_PointLightsIntensity[MAX_NUM_POINT];
uniform float u_PointLightsAttenRangeInv[MAX_NUM_POINT];
#endif
#endif

//SpotLight
#ifdef AE_SpotLightNum
#if AE_SpotLightNum
uniform float u_SpotLightsEnabled[MAX_NUM_SPOT];
uniform vec3 u_SpotLightsDirection[MAX_NUM_SPOT];
uniform vec3 u_SpotLightsPosition[MAX_NUM_SPOT];
uniform vec3 u_SpotLightsColor[MAX_NUM_SPOT];
uniform float u_SpotLightsIntensity[MAX_NUM_SPOT];
uniform float u_SpotLightsInnerAngleCos[MAX_NUM_SPOT];
uniform float u_SpotLightsOuterAngleCos[MAX_NUM_SPOT];
uniform float u_SpotLightsAttenRangeInv[MAX_NUM_SPOT];
#endif
#endif

uniform vec3 u_UVoffset;
uniform float u_MinDistToSurface;

uniform float u_FurLength;
uniform float u_LightMoveFactor;

#ifdef FurBend
uniform float u_ForcePow;
uniform vec3 u_Force;
#endif


struct VSBuffer
{
    vec4 homogeneous_pos;
    vec3 model_position;
    vec3 model_normal;
    vec3 model_tangent;
    vec3 model_binormal;
    vec3 local_position;
    vec2 texcoord;
    float elapsed_time;
    vec4 model_color;
};

struct ShadingBuffer
{
    int shading_model_id;
    vec3 pos;
    vec3 n;
    vec3 furN;
    vec3 t;
    vec3 bn;
    vec3 v;
    vec3 l;
    vec3 r;
    vec3 diffuse;
    vec3 specular;
    float metallic;
    float roughness;
    vec4 custom1;
    vec4 custom2;
    vec4 custom3;
    vec4 custom4;
};

struct VsLightProperties
{
    int dir_light_num;
    float dir_light_enabled[MAX_NUM_DIR];
    vec3 dir_light_dir[MAX_NUM_DIR];
    vec3 dir_light_color[MAX_NUM_DIR];
    float dir_light_intensity[MAX_NUM_DIR];
    float dir_light_shadow_factor;

    int point_light_num;
    float point_light_enabled[MAX_NUM_POINT];
    vec3 point_light_pos[MAX_NUM_POINT];
    vec3 point_light_color[MAX_NUM_POINT];
    float point_light_intensity[MAX_NUM_POINT];
    float point_light_attenuation_rcp[MAX_NUM_POINT];

    int spot_light_num;
    float spot_light_enabled[MAX_NUM_SPOT];
    vec3 spot_light_dir[MAX_NUM_SPOT];
    vec3 spot_light_pos[MAX_NUM_SPOT];
    vec3 spot_light_color[MAX_NUM_SPOT];
    float spot_light_intensity[MAX_NUM_SPOT];
    float spot_light_inner_cos[MAX_NUM_SPOT];
    float spot_light_outer_cos[MAX_NUM_SPOT];
    float spot_light_attenuation_rcp[MAX_NUM_SPOT];
};

float hash(float n)
{
    return fract(sin(n) * 43758.5453);
}

float noise(vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 57.0 + 113.0 * p.z;
    return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x), mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0), f.x), mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
}

void VsLightPropertiesToDefault(inout VsLightProperties l)
{
    l.dir_light_num = 0;
    l.dir_light_shadow_factor = 1.0;
    l.dir_light_enabled[0] = 0.0;
    l.dir_light_intensity[0] = 0.0;

    l.point_light_num = 0;

    l.spot_light_num = 0;
}

float StrandSpecular(vec3 T, vec3 H, float exponent)
{
    // float cosTH = dot(T, H);
    // float sinTH = sqrt(1.0 - cosTH * cosTH);
    // float dirAtten = smoothstep(-1.0, 0.0, cosTH);
    // return clamp(dirAtten * pow(sinTH, exponent), 0.0, 1.0);
    float cosTH = dot(T, H);
    float sinTH = sqrt(1.0 - cosTH * cosTH);
    return  clamp(pow(sinTH, exponent), 0.0, 1.0);
}

// shadingmodel
vec3 StandardShading(vec3 N,vec3 T,vec3 V, vec3 L, vec3 FurN ,vec3 lightFactor)
{
    vec3 H = normalize(L + V);
    float NoL = dot(N, L) * 0.5 +0.5;
    float Vis = clamp(NoL + _FURLEVEL * u_LightFilter, 0.0, 1.0);
    varDiffuse += Vis * lightFactor;

    #ifdef StrandSpec
        vec2 strandExp = vec2(255.0 - u_StrandExpHigh, 25.0 - u_StrandExpLow);
        vec3 T1 = normalize(u_StrandShift.x * FurN + T);
        vec3 T2 = normalize(u_StrandShift.y * FurN + T);

        float strandSpec1 = StrandSpecular(T1, H, strandExp.x );
        float strandSpec2 = StrandSpecular(T2, H, strandExp.y );
        vec3 strandSpecHigh = strandSpec1 * u_StrandSpecColor.xyz * u_Roughness;
        vec3 strandSpecLow = strandSpec2 * u_StrandSpecColor_low.xyz * min(u_Roughness,1.0);
        vec3 strandSpecSum = (strandSpecHigh + strandSpecLow) * lightFactor;

        varStrandSpecular += strandSpecSum * (NoL*0.5 +0.5);
    #endif
    return varDiffuse;
}

vec3 SurfaceShading(in ShadingBuffer sb, vec3 lightFactor)
{
    if (sb.shading_model_id == FUR_MODEL_ID)
    {
        return StandardShading(sb.n,sb.t,sb.v, sb.l,sb.furN, lightFactor);
    }
}

// lightmodel
void DLShading(in ShadingBuffer sb, in VsLightProperties l)
{
#ifdef AE_DirLightNum
#if AE_DirLightNum
    sb.l = l.dir_light_dir[0];
    SurfaceShading(sb, l.dir_light_intensity[0] * l.dir_light_color[0] * l.dir_light_enabled[0]);
#if AE_DirLightNum - 1
    sb.l = l.dir_light_dir[1];
    SurfaceShading(sb, l.dir_light_intensity[1] * l.dir_light_color[1] * l.dir_light_enabled[1]);
#if AE_DirLightNum - 2
    sb.l = l.dir_light_dir[2];
    SurfaceShading(sb, l.dir_light_intensity[2] * l.dir_light_color[2] * l.dir_light_enabled[2]);
#endif
#endif
#endif
#endif
}

void PLShading(in ShadingBuffer sb, in VsLightProperties l)
{
#ifdef AE_PointLightNum
#if AE_PointLightNum
    vec3 light_vec1 = (l.point_light_pos[0] - sb.pos) * l.point_light_attenuation_rcp[0];
    float light_dist1 = length(light_vec1);
    sb.l = light_vec1 / light_dist1;
    float nl1 = max(0.0, dot(sb.n, sb.l));
    float fall_off1 = pow(clamp(1.0 - pow(light_dist1 / 1.0, 4.0), 0.0, 1.0), 2.0) * PI / (4.0 * PI) * (pow(light_dist1, 2.0) + 1.0) * nl1;
    SurfaceShading(sb, l.point_light_intensity[0] * l.point_light_color[0] * fall_off1 * l.point_light_enabled[0]);
#if AE_PointLightNum - 1
    vec3 light_vec2 = (l.point_light_pos[1] - sb.pos) * l.point_light_attenuation_rcp[1];
    float light_dist2 = length(light_vec2);
    sb.l = light_vec2 / light_dist2;
    float nl2 = max(0.0, dot(sb.n, sb.l));
    float fall_off2 = pow(clamp(1.0 - pow(light_dist2 / 1.0, 4.0), 0.0, 1.0), 2.0) * PI / (4.0 * PI) * (pow(light_dist2, 2.0) + 1.0) * nl2;
    SurfaceShading(sb, l.point_light_intensity[1] * l.point_light_color[1] * fall_off2 * l.point_light_enabled[1]);
#endif
#endif
#endif
}

void SLShading(in ShadingBuffer sb, in VsLightProperties l)
{
#ifdef AE_SpotLightNum
#if AE_SpotLightNum
    vec3 light_vec = (l.spot_light_pos[0] - sb.pos) * l.spot_light_attenuation_rcp[0];
    float light_dist = length(light_vec);
    sb.l = light_vec / light_dist;
    float nl = max(0.0, dot(sb.n, sb.l));
    float fall_off = pow(clamp(1.0 - pow(light_dist / 1.0, 4.0), 0.0, 1.0), 2.0) * PI / (4.0 * PI) * (pow(light_dist, 2.0) + 1.0) * nl;
    float c = max(0.0, dot(sb.l, l.spot_light_dir[0]));
    fall_off *= smoothstep(l.spot_light_outer_cos[0], l.spot_light_inner_cos[0], c);
    SurfaceShading(sb, l.spot_light_intensity[0] * l.spot_light_color[0] * fall_off * l.spot_light_enabled[0]);
#if AE_SpotLightNum - 1
    vec3 light_vec2 = (l.spot_light_pos[1] - sb.pos) * l.spot_light_attenuation_rcp[1];
    float light_dist2 = length(light_vec2);
    sb.l = light_vec2 / light_dist2;
    float nl2 = max(0.0, dot(sb.n, sb.l));
    float fall_off2 = pow(clamp(1.0 - pow(light_dist / 1.0, 4.0), 0.0, 1.0), 2.0) * PI / (4.0 * PI) * (pow(light_dist, 2.0) + 1.0) * nl2;
    float c2 = max(0.0, dot(sb.l, l.spot_light_dir[1]));
    fall_off2 *= smoothstep(l.spot_light_outer_cos[1], l.spot_light_inner_cos[1], c2);
    SurfaceShading(sb, l.spot_light_intensity[1] * l.spot_light_color[1] * fall_off2 * l.spot_light_enabled[1]);
#endif
#endif
#endif
}
// lightshading
float Shadowing(in ShadingBuffer sb)
{
    return 1.0;
}

void SetLightWithShadow(in ShadingBuffer sb, inout VsLightProperties l, bool hasShadow)
{
#ifdef AE_DirLightNum
#if AE_DirLightNum
    l.dir_light_dir[0] = normalize(-lightDir[0].xyz);
    l.dir_light_color[0] = lightColors[0].xyz;
    l.dir_light_intensity[0] = lightIntensity;
    l.dir_light_enabled[0] = 1.0;
    if (hasShadow)
    {
        l.dir_light_shadow_factor = Shadowing(sb);
    }
#if AE_DirLightNum - 1
    l.dir_light_dir[1] = normalize(-u_DirLightsDirection[1]);
    l.dir_light_color[1] = u_DirLightsColor[1];
    l.dir_light_intensity[1] = u_DirLightsIntensity[1];
    l.dir_light_enabled[1] = u_DirLightsEnabled[1];
#endif
#if AE_DirLightNum - 2
    l.dir_light_dir[2] = normalize(-u_DirLightsDirection[2]);
    l.dir_light_color[2] = u_DirLightsColor[2];
    l.dir_light_intensity[2] = u_DirLightsIntensity[2];
    l.dir_light_enabled[2] = u_DirLightsEnabled[2];
#endif
#endif
#endif

#ifdef AE_PointLightNum
#if AE_PointLightNum
    l.point_light_pos[0] = u_PointLightsPosition[0];
    l.point_light_color[0] = u_PointLightsColor[0];
    l.point_light_intensity[0] = u_PointLightsIntensity[0];
    l.point_light_attenuation_rcp[0] = u_PointLightsAttenRangeInv[0];
    l.point_light_enabled[0] = u_PointLightsEnabled[0];
#if AE_PointLightNum - 1
    l.point_light_pos[1] = u_PointLightsPosition[1];
    l.point_light_color[1] = u_PointLightsColor[1];
    l.point_light_intensity[1] = u_PointLightsIntensity[1];
    l.point_light_attenuation_rcp[1] = u_PointLightsAttenRangeInv[1];
    l.point_light_enabled[1] = u_PointLightsEnabled[1];
#endif
#endif
#endif

#ifdef AE_SpotLightNum
#if AE_SpotLightNum
    l.spot_light_dir[0] = normalize(-u_SpotLightsDirection[0]);
    l.spot_light_pos[0] = u_SpotLightsPosition[0];
    l.spot_light_color[0] = u_SpotLightsColor[0];
    l.spot_light_intensity[0] = u_SpotLightsIntensity[0];
    l.spot_light_inner_cos[0] = u_SpotLightsInnerAngleCos[0];
    l.spot_light_outer_cos[0] = u_SpotLightsOuterAngleCos[0];
    l.spot_light_attenuation_rcp[0] = u_SpotLightsAttenRangeInv[0];
    l.spot_light_enabled[0] = u_SpotLightsEnabled[0];
#if AE_SpotLightNum - 1
    l.spot_light_dir[1] = normalize(-u_SpotLightsDirection[1]);
    l.spot_light_pos[1] = u_SpotLightsPosition[1];
    l.spot_light_color[1] = u_SpotLightsColor[1];
    l.spot_light_intensity[1] = u_SpotLightsIntensity[1];
    l.spot_light_inner_cos[1] = u_SpotLightsInnerAngleCos[1];
    l.spot_light_outer_cos[1] = u_SpotLightsOuterAngleCos[1];
    l.spot_light_attenuation_rcp[1] = u_SpotLightsAttenRangeInv[1];
    l.spot_light_enabled[1] = u_SpotLightsEnabled[1];
#endif
#endif
#endif
}

void CalcLightShading(in ShadingBuffer sb, in VsLightProperties l)
{
    DLShading(sb, l);
    PLShading(sb, l);
    SLShading(sb, l);
}
vec3 direction(vec3 positionOffset, vec3 normal, vec3 dir, float FurLength)
{
    if (length(positionOffset) == 0.0) 
        return dir;

    vec3 sidedir = normalize(cross(dir, positionOffset));
    vec3 updir = normalize(cross(sidedir, dir));

    vec3 side = normalize(cross(normal, updir));
    vec3 up = normalize(cross(side, normal));


    float cosTheta = dot(dir, normalize(positionOffset));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float sinBeta = (length(positionOffset) * sinTheta) / FurLength;
    float tanBeta = sinBeta / sqrt(1.0 - sinBeta * sinBeta);
    float upBias = FurLength * tanBeta * _FURLEVEL;

    //if(dot(normal,updir) < 0.0)
        //updir = up;

    vec3 outdir = normalize(dir * FurLength * _FURLEVEL + updir * upBias);

    mat3 upToLocal = mat3(up.x,up.y,up.z,
                    side.x,side.y,side.z,
                    normal.x,normal.y,normal.z);

    vec3 outdirUpSpace = vec3(dot(outdir,up),dot(outdir,side),dot(outdir,normal));
    outdirUpSpace.z = max(outdirUpSpace.z, u_MinDistToSurface);
    outdir = normalize(upToLocal * outdirUpSpace);
    return outdir;
}

void main()
{
    
    VSBuffer VB;
    VB.homogeneous_pos = vec4(position, 1.0);
    VB.model_position = position;
    VB.local_position = position;
   // VB.model_tangent = normalize(attTangent * u_UVoffset.x - attBinormal * u_UVoffset.y);
    VB.model_normal = normal;


    VB.texcoord = texCoords;
    VB.elapsed_time = u_Time.y;

    vec3 positionOffset = vec3(0.0, 0.0000000001, 0.0);
    vec3 rotationOffset = vec3(0.0, 0.0000000001, 0.0);
    vec3 centorPos = vec3(0.0, 0.0000000001, 0.0);
    vec3 linearVelocity = vec3(0.0, 0.0000000001, 0.0);
    vec3 angularVelocity = vec3(0.0, 0.0000000001, 0.0);

    vec3 origWorldNormal = vec3(0.0);

//#ifdef AMAZING_USE_BONES
//    mat4 boneTransform = u_Palatte[int(attBoneIds.x)] * attWeights.x;
//    boneTransform += u_Palatte[int(attBoneIds.y)] * attWeights.y;
//    boneTransform += u_Palatte[int(attBoneIds.z)] * attWeights.z;
//    boneTransform += u_Palatte[int(attBoneIds.w)] * attWeights.w;
//
//    vec3 bm_postiton = (boneTransform * vec4(VB.model_position, 1.0)).xyz;
//    vec3 bn_normal = (boneTransform * vec4(VB.model_normal, 0.0)).xyz;
//    varWorldPosition = (u_Model * vec4(bm_postiton, 1.0)).xyz;
//    varWorldNormal = (u_TransposeInvModel * vec4(bn_normal, 0.0)).xyz;
//    VB.homogeneous_pos = vec4(bm_postiton, 1.0);
//    VB.model_position = bm_postiton;
//    VB.model_normal = bn_normal;
//
//#else
//    varWorldPosition = (u_Model * vec4(VB.model_position, 1.0)).xyz;
//    varWorldNormal = (u_TransposeInvModel * vec4(VB.model_normal, 0.0)).xyz;
//
//#endif

	mat4 modelToWorld = mat4(1.0);

	if (AnimationEnable)
	{
		int i0 = int(gBoneIDsArray[0]);
        int i1 = int(gBoneIDsArray[1]);
        int i2 = int(gBoneIDsArray[2]);
        int i3 = int(gBoneIDsArray[3]);
        mat4 BoneTransform = gBonesMatrix[i0] * gBoneWeightsArray[0];	
        BoneTransform += gBonesMatrix[i1] * gBoneWeightsArray[1];
        BoneTransform += gBonesMatrix[i2] * gBoneWeightsArray[2];
        BoneTransform += gBonesMatrix[i3] * gBoneWeightsArray[3];
        modelToWorld =  BoneTransform * inverse(meshMat);
	}
	modelToWorld = world * modelToWorld * meshMat;

		//normal
	vec4 n = vec4(normal, 0.0);
	n =  modelToWorld * n;
	vec4 ns = vec4(0.0, 0.0, 0.0, 0.0);
	ns = modelToWorld * ns;
	vec3 worldNormal = normalize(n - ns).xyz;

	vec4 PosW = modelToWorld * vec4(position, 1.0);
	varWorldPosition = PosW.xyz / PosW.w;
	varWorldNormal = worldNormal;

    varWorldNormal = normalize(varWorldNormal);
    float FurLength = u_FurLength;
    origWorldNormal = varWorldNormal;
    float NoL = dot(varWorldNormal,normalize(-lightDir[0].xyz));
    float lightAtten = smoothstep(-mix((_FURLEVEL), 1.0, Fur_Params.w) * 0.5, 0.5, NoL);
    vec3 dir =  varWorldNormal;


    vec3 up, side;
    float noiseBias = 0.0;
    float angle = 0.0;
#ifdef FurAllBend
#ifdef FurBend
    if (length(u_Force) > 0.000001)
    {
        vec3 force = u_Force;
        side = normalize(cross(varWorldNormal, force));
        up = normalize(cross(side, varWorldNormal));
        float cosTheta = dot(varWorldNormal, normalize(force));
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
        //float upBias = 0.3 * u_ForcePow * sinTheta * _FURLEVEL * _FURLEVEL * (3.0 - _FURLEVEL) * length(force);
        float upBias = sinTheta * length(force) * pow(_FURLEVEL, 10.0 - u_ForcePow) * (1.5 - _FURLEVEL);

        dir = normalize(dir * FurLength * _FURLEVEL + up * (upBias + noiseBias));
    }
#endif
#endif
    if (length(dir) < 0.01)
        dir = varWorldNormal;
    else
        dir = normalize(dir);
    vec3 offset = dir * _FURLEVEL * FurLength;
	outDir = offset;
    // varWorldNormal = dir;
    varTexcoord0 = VB.texcoord;

	vec3 direction = mix( normal, vGravity * vGravityStength + normal * (1.0 - vGravityStength), _FURLEVEL );
	vec3 P = position + direction * FurLength * _FURLEVEL;
	gl_Position = projection * view * modelToWorld * vec4(P, 1.0);

    //gl_Position = projection * (view * PosW + view * vec4(offset * 100.0, 0.0));

    // Data used in lighting calculations.
    ShadingBuffer SB;
    SB.shading_model_id = FUR_MODEL_ID;
    SB.pos = PosW.xyz + offset;

    SB.t = normalize(dir);
    SB.n = varWorldNormal;

    vec3 furSide = normalize(cross(SB.t, vec3(0.0, 1.0, 0.0)));
    vec3 furUp = normalize(cross(furSide, SB.t));
    SB.furN = mix(SB.n,furUp,step(0.8,dot(SB.t,SB.n)));
    SB.v = normalize(camPos - SB.pos);
    vec4 empiricalParameter = vec4(0.5,0.39,0.25,1.0)*0.95;

    varDiffuse = vec3(0.0);
    varStrandSpecular = vec3(0.0);
    // varFabricSpecular = vec3(0.0);

    float aoFactor = clamp(_FURLEVEL*1.3333  + 0.2, 0.0, 1.0);
    varAO = mix(u_AOColor.rgb, vec3(1.0), aoFactor);
    varRim = 0.0;
    float fresnel = 1.0 - max(0.0, dot(origWorldNormal, SB.v));
#ifdef FurRim
      varRim = fresnel * fresnel * mix(clamp(NoL,0.0,1.0),1.0,0.3)* pow(_FURLEVEL,1.5);
    varRim = varRim * varRim * u_RimIntensity;
#endif

    // light
    VsLightProperties LP;
    VsLightPropertiesToDefault(LP);
    SetLightWithShadow(SB, LP, true);
    CalcLightShading(SB, LP);

    // #ifdef SingleFurShadow
    //     // vec3 WorldBinormal  = (normalize(u_TransposeInvModel * vec4(attBinormal, 0.0))).xyz;
    //     // mat3 TBN = mat3(SB.t, WorldBinormal, SB.n);
    //     // vec3 modelLightDir = (u_InvModel * vec4(normalize(-u_DirLightsDirection[0]),0.0)).xyz;
    //     // varLightUVoffset = (TBN * modelLightDir).xy;
    //     // varLightUVoffset = u_ShadowDir;
    // #endif 

    varStrandSpecular =varStrandSpecular* lightAtten * pow(_FURLEVEL, Fur_Spec_Param.y) * mix(vec3(1.0), empiricalParameter.xyz, Fur_Spec_Param.x);
  
}
