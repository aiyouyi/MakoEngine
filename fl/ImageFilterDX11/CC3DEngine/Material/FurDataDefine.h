#pragma once
#include "Toolbox/Render/CC3DShaderDef.h"

//struct FurConstBuffer
//{
//	Vector3 vForce = { 0, -0.1, 0 };
//	float furOffset = 0.0f;
//
//	float furLength = 0.1;
//	float uvScale = 20.0f;
//	float lightFilter = 0.0f;
//	float furLightExposure = 1.0f;
//
//	float fresnelLV = 1.0f;
//	float furMask = 0.5f;
//	float tming = 0.5f;
//	int drawSolid = 0;
//
//	Vector3 furColor = Vector3(1.0f, 1.0f, 1.0f);
//	float furGamma = 1.0f;
//	int useToneMapping = 0;
//	int useLengthTex = 0;
//	Vector2 padding;
//};

BEGIN_SHADER_STRUCT(FurConstBuffer, 2)
	DECLARE_PARAM(Vector3, vGravity)
	DECLARE_PARAM_VALUE(float, FurOffset,0.f)
	DECLARE_PARAM_VALUE(float, FurLength, 0.0f)
	DECLARE_PARAM_VALUE(float, UVScale, 1.f)
	DECLARE_PARAM_VALUE(float, LightFilter, 0.0f)
	DECLARE_PARAM_VALUE(float, FurLightExposure, 1.0f)
	DECLARE_PARAM_VALUE(float, FresnelLV, 1.0f)
	DECLARE_PARAM_VALUE(float, FurMask, 0.5f)
	DECLARE_PARAM_VALUE(float, Tming, 0.5f)
	DECLARE_PARAM_VALUE(int, DrawSolid, 0)
	DECLARE_PARAM_VALUE(Vector3, FurColor, Vector3(1.0f, 1.0f, 1.0f))
	DECLARE_PARAM_VALUE(float, FurGamma, 1.0f)
	DECLARE_PARAM_VALUE(int, UseToneMapping, 0)
	DECLARE_PARAM_VALUE(int, UseLengthTex, 0)
	DECLARE_PARAM_VALUE(float, FurAmbientStrength, 2.0f)
	DECLARE_PARAM_VALUE(float, padding, 0.0f)
	BEGIN_STRUCT_CONSTRUCT(FurConstBuffer)
		IMPLEMENT_PARAM("vGravity", UniformType::FLOAT3)
		IMPLEMENT_PARAM("FurOffset", UniformType::FLOAT)
		IMPLEMENT_PARAM("FurLength", UniformType::FLOAT)
		IMPLEMENT_PARAM("UVScale", UniformType::FLOAT)
		IMPLEMENT_PARAM("LightFilter", UniformType::FLOAT)
		IMPLEMENT_PARAM("FurLightExposure", UniformType::FLOAT)
		IMPLEMENT_PARAM("FresnelLV", UniformType::FLOAT)
		IMPLEMENT_PARAM("FurMask", UniformType::FLOAT)
		IMPLEMENT_PARAM("Tming", UniformType::FLOAT)
		IMPLEMENT_PARAM("DrawSolid", UniformType::INT)
		IMPLEMENT_PARAM("FurColor", UniformType::FLOAT3)
		IMPLEMENT_PARAM("FurGamma", UniformType::FLOAT)
		IMPLEMENT_PARAM("UseToneMapping", UniformType::INT)
		IMPLEMENT_PARAM("UseLengthTex", UniformType::INT)
		IMPLEMENT_PARAM("FurAmbientStrength", UniformType::FLOAT)
		IMPLEMENT_PARAM("padding", UniformType::FLOAT)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT



BEGIN_SHADER_STRUCT(BrightFurConstBuffer, 2)
	DECLARE_PARAM_VALUE(Vector4, AOColor, Vector4(0.46667, 0.45882, 0.45882, 1.0))
	DECLARE_PARAM_VALUE(Vector4, StrandSpecColor, Vector4(0.90196, 0.89804, 1.0, 1.0))
	DECLARE_PARAM_VALUE(Vector4, StrandSpecColor_low, Vector4(0.63922, 0.38824, 0.76471, 1.0))
	DECLARE_PARAM_VALUE(Vector4, EnvironmentColor, Vector4( 0.87843, 0.80392, 0.79608, 1.0 ))
	DECLARE_PARAM_VALUE(Vector4, FurColor, Vector4(1.0, 1.0, 1.0, 1.0))
	DECLARE_PARAM_VALUE(Vector3, Force, Vector3(0.0, 2.0, 0.0))
	DECLARE_PARAM_VALUE(Vector3, UVOffset, Vector3(0.0, 0.0, 0.0))
	DECLARE_PARAM_VALUE(Vector3, vGravity, Vector3(0.0, -0.01, 0.0))
	DECLARE_PARAM_VALUE(Vector2, StrandShift, Vector2(-4.0, 5.0))
	DECLARE_PARAM_VALUE(float, LightFilter, 6.8f)
	DECLARE_PARAM_VALUE(float, Roughness, 0.7f)
	DECLARE_PARAM_VALUE(float, StrandExpHigh, 224.4f)
	DECLARE_PARAM_VALUE(float, StrandExpLow, 14.75)
	DECLARE_PARAM_VALUE(float, FurLength, 0.2f)
	DECLARE_PARAM_VALUE(float, lightIntensity, 1.0f)
	DECLARE_PARAM_VALUE(float, ForcePow, 9.0f)
	DECLARE_PARAM_VALUE(float, IBLExposure, 1.45f)
	DECLARE_PARAM_VALUE(float, FurMaskScale, 6.0f)
	DECLARE_PARAM_VALUE(float, FurThickness, 0.17f)
	DECLARE_PARAM_VALUE(float, FurThickness_B, 8.901)
	DECLARE_PARAM_VALUE(float, FurThickness_T, 1.19)
	DECLARE_PARAM_VALUE(float, FurThickness_B2T, 1.672)
	DECLARE_PARAM_VALUE(float, FurFeather, 1.672)
	DECLARE_PARAM_VALUE(float, _FURLEVEL, 0.1)
	DECLARE_PARAM_VALUE(int, drawSolid, 0)
	DECLARE_PARAM_VALUE(float, padding, 0.0)
	BEGIN_STRUCT_CONSTRUCT(BrightFurConstBuffer)
		IMPLEMENT_PARAM("u_AOColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("u_StrandSpecColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("u_StrandSpecColor_low", UniformType::FLOAT4)
		IMPLEMENT_PARAM("u_EnvironmentColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("u_FurColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("u_Force", UniformType::FLOAT3)
		IMPLEMENT_PARAM("u_UVoffset", UniformType::FLOAT3)
		IMPLEMENT_PARAM("vGravity", UniformType::FLOAT3)
		IMPLEMENT_PARAM("u_StrandShift", UniformType::FLOAT2)
		IMPLEMENT_PARAM("u_LightFilter", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_Roughness", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_StrandExpHigh", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_StrandExpLow", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurLength", UniformType::FLOAT)
		IMPLEMENT_PARAM("lightIntensity", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_ForcePow", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_IBLExposure", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurMaskScale", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurThickness", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurThickness_B", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurThickness_T", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurThickness_B2T", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_FurFeather", UniformType::FLOAT)
		IMPLEMENT_PARAM("_FURLEVEL", UniformType::FLOAT)
		IMPLEMENT_PARAM("drawSolid", UniformType::INT)
		IMPLEMENT_PARAM("padding", UniformType::FLOAT)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT