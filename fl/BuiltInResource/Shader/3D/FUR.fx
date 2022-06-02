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
    float pad;
};
Texture2D colorMap : register(t0);
Texture2D noiseMap : register(t1);
Texture2D lengthMap : register(t2);
Texture2D lengthMapVS : register(t3);
SamplerState samLinear : register(s0);
SamplerState samLinearVS : register(s1);


static const float PI = 3.14159265359;
struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4  BlendIndices  : BLENDINDICES;
	float4  BlendWeights  : BLENDWEIGHT;

};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float3 normal: NORMAL0; 
    float3 worldPos	: POSITION;
	float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float3 SH : NORMAL1;
};

static const int MAX_MATRICES = 200;
cbuffer ConstantBuffer : register(b1)
{
	matrix BoneMat[MAX_MATRICES];
}

cbuffer ConstantBuffer : register(b2)
{
    float3 vGravity;
    float FurOffset;
    float FurLength;
    float UVScale;
    float _LightFilter;
    float _FurLightExposure;
    float _FresnelLV;
    float _FurMask;
    float _tming;
    int drawSolid;
	float3 furColor;
	float furGamma;
	int useToneMapping;
	int useLengthTex;
	float2 padding;
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
	Color = max(float3(6.10352e-5, 6.10352e-5, 6.10352e-5), Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
	return float3(SrgbToLinearChannel(Color.r), SrgbToLinearChannel(Color.g), SrgbToLinearChannel(Color.b));
}

float3 ToneMapping(float3 Color)
{
	return LinearToSrgb(ACESFilm(Color));
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
	ModeltoWorld = mul(ModeltoWorld, world);

    float2 uvoffset = float2(0.2, 0.2) * FurOffset;
    uvoffset *= 0.1;

    output.Tex0 = input.Tex * UVScale + uvoffset;
    output.Tex1 = input.Tex;
	float furLength_coeff = 1.0;
	if (useLengthTex == 1)
	{
		furLength_coeff = lengthMapVS.SampleLevel(samLinearVS, output.Tex1, 0).a;
		//float4 lengthCof = lengthMapVS[output.Tex1];
		//furLength_coeff = lengthCof.a;
	}
    //float3 P = input.Pos + input.normal * FurLength * FurOffset * furLength_coeff;
    //P += vGravity * pow(FurOffset, 3) * FurLength;
	float vGravityStength = 0.5;
	float3 direction = lerp(input.normal, vGravity * vGravityStength + input.normal * (1.0 - vGravityStength), FurOffset);
	float3 P = input.Pos + direction * FurLength * FurOffset * furLength_coeff;

	// float4 PosL = mul(float4(input.Pos, 1.0), ModeltoWorld);
    // matrix ViewProj = mul(view, projection);
	// output.Pos = mul(PosL, ViewProj);

	// output.worldPos = PosL.xyz/PosL.w;
    // output.Tex = input.Tex;


	float4 n = float4(input.normal, 0.0);
	n = mul(n, ModeltoWorld);
	float4 ns = float4(0.0, 0.0, 0.0, 0.0);
	ns = mul(ns, ModeltoWorld);
	output.normal = normalize(n - ns).xyz;
    output.normal.y = -output.normal.y;

    float4 view_normal = normalize( mul(output.normal, view));
	float _sh = clamp(output.normal.y * 0.25 + 0.35, 0.0, 1.0);
    output.SH = float3(_sh, _sh, _sh );

    float4 PosL = mul(float4(P, 1.0), ModeltoWorld);
    matrix ViewProj = mul(view, projection);
	output.Pos = mul(PosL, ViewProj);
    output.worldPos = PosL.xyz/PosL.w;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    if(drawSolid == 1)
    {
        return colorMap.Sample(samLinear, input.Tex1);
    }
	
	//return float4(input.SH, 1.0);

    float3 baseColor = colorMap.Sample( samLinear, input.Tex1 ).rgb;
	if (useToneMapping == 1)
	{
		baseColor = sRGBToLinear(baseColor);
	}	
	float Noise = 1.0;
	if (useLengthTex == 1)
	{
		float3 NoiseMask = noiseMap.Sample(samLinear, input.Tex0).rgb;
		float3 RegionMask = lengthMap.Sample(samLinear, input.Tex1).rgb;
		Noise = NoiseMask.r * RegionMask.r + NoiseMask.g * RegionMask.g + NoiseMask.b * RegionMask.b;
	}
	else
	{
		Noise = noiseMap.Sample(samLinear, input.Tex0).r;
	}

    //float Noise = noiseMap.Sample(samLinear, input.Tex0).r;

    //Ambient occlusion
    float Occlusion = FurOffset * FurOffset;
    Occlusion += 0.04;
    float3 SHL = lerp( furColor * input.SH, input.SH, Occlusion );

    ////次表面散射（太阳光）
    
    float NoL = dot(lightDir[0].xyz, input.normal ) * 0.5 + 0.5;
    float DirLight = clamp( NoL + _LightFilter + FurOffset, 0.0, 1.0 );
    DirLight *= _FurLightExposure * ambientStrength;

    //轮廓光
    float3 V = normalize( CamPos - input.worldPos );
    float Fresnel = 1.0 - max( 0.0, dot(input.normal, V) );
    float3 RimLight = float3( Fresnel * Occlusion, Fresnel * Occlusion, Fresnel * Occlusion); //这个值会很小，因为Occlusion太小了，所以对最终效果影响比较小
    RimLight *= RimLight;
    RimLight *= _FresnelLV * input.SH * baseColor;
    SHL += RimLight;

    float alpha = clamp((Noise * 2.0 -(FurOffset * FurOffset +(FurOffset*_FurMask*5.0)))*_tming, 0.0, 1.0);

	float3 outColor = DirLight * baseColor + SHL;

	if (useToneMapping == 1)
	{
		outColor = ToneMapping(pow(outColor, float3(furGamma, furGamma, furGamma)));
	}

    return float4(outColor, alpha );

    //return float4( normalize(input.normal) * 0.5 + 0.5, 1.0 );
}