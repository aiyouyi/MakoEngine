#pragma once
#ifndef _H_CC3D_BRIGHT_FUR_TEXTURE_H_
#define _H_CC3D_BRIGHT_FUR_TEXTURE_H_

#include <unordered_map>

#include "CC3DMaterialGL.h"
//#include "ToolBox/GL/CCProgram.h"
#include "BaseDefine/Vectors.h"
#include "CC3DEngine/Material/FurDataDefine.h"
#include "Render/MaterialTexRHI.h"

class CC3DMesh;

class CC3DBrightFurMaterial : public CC3DMaterialGL
{
public:
	CC3DBrightFurMaterial();
	virtual ~CC3DBrightFurMaterial();

	virtual void InitShaderProgram(std::string path) override;

	virtual void RenderSet(CC3DMesh* pMesh) override;

	virtual void PreRenderSet(CC3DMesh* pMesh);

	virtual void LoadConfig(const std::string config_file);

	virtual void LoadEnvironmentConfig() override;

	DELCARE_SHADER_STRUCT_MEMBER(BrightFurConstBuffer);
public:
	Vector4 AOColor = {0.46667, 0.45882, 0.45882, 1.0};
	float LightFilter = 6.8f;
	float Roughness = 0.7f;
	Vector4 StrandSpecColor = { 0.90196, 0.89804, 1.0, 1.0 };
	float StrandExpHigh = 224.4f;
	float StrandExpLow = 14.75f;
	Vector4 StrandSpecColor_low = { 0.63922, 0.38824, 0.76471, 1.0 };
	Vector2 StrandShift = {-4.0, 5.0};
	float MinDistToSurface;
	float FurLength = 0.2f;
	float lightIntensity = 1.0f;
	//float FurLevel;
	float ForcePow = 9.0f;
	Vector3 Force = {0.0, 2.0, 0.0};
	Vector4 EnvironmentColor = {0.87843, 0.80392, 0.79608, 1.0};
	float IBLExposure = 1.45f;
	Vector4 FurColor = {1.0, 1.0, 1.0, 1.0};
	float FurMaskScale = 6.0f;
	float FurThickness = 0.17f;
	float FurThickness_B = 8.901;
	float FurThickness_T = 1.19f;
	float FurThickness_B2T = 1.672f;
	float FurFeather = 1.0f;
	Vector3 UVOffset = {0.0, 0.0, 0.0};
	Vector3 vGravity = { 0.0, -0.01, 0.0 };

	int numLayers = 10;

	std::string noise_tex_file;
	std::string default_tex_file;

	std::shared_ptr<MaterialTexRHI> noiseTexture = nullptr;
};

#endif
