#pragma once
#ifndef _H_CC3D_EYE_TEXTURE_H_
#define _H_CC3D_EYE_TEXTURE_H_

#include <unordered_map>

#include "Material/CC3DMaterial.h"
//#include "ToolBox/GL/CCProgram.h"
#include "Toolbox/Render/CC3DShaderDef.h"

BEGIN_SHADER_STRUCT(EyeDoll, 2)
	DECLARE_PARAM(Vector3, specoffset1)
	DECLARE_PARAM(Vector3, specoffset2)
	DECLARE_PARAM(Vector3, specoffset3)
	DECLARE_PARAM(Vector3, initoffset)
	DECLARE_PARAM(Vector3, intensity)
	DECLARE_PARAM_VALUE(float, movespeed1, 0.5)
	DECLARE_PARAM_VALUE(float, movespeed2, 0.5)
	DECLARE_PARAM_VALUE(float, movespeed3, 0.5)
	DECLARE_PARAM_VALUE(float, eyeheight, 0.5)
	BEGIN_STRUCT_CONSTRUCT(EyeDoll)
		IMPLEMENT_PARAM("specoffset1", UniformType::FLOAT3)
		IMPLEMENT_PARAM("specoffset2", UniformType::FLOAT3)
		IMPLEMENT_PARAM("specoffset3", UniformType::FLOAT3)
		IMPLEMENT_PARAM("initoffset", UniformType::FLOAT3)
		IMPLEMENT_PARAM("intensity", UniformType::FLOAT3)
		IMPLEMENT_PARAM("movespeed1", UniformType::FLOAT)
		IMPLEMENT_PARAM("movespeed2", UniformType::FLOAT)
		IMPLEMENT_PARAM("movespeed3", UniformType::FLOAT)
		IMPLEMENT_PARAM("eyeheight", UniformType::FLOAT)
	INIT_TEXTURE_INDEX("heightMap", 0)
	INIT_TEXTURE_INDEX("specularMap", 1)
	INIT_TEXTURE_INDEX("heightMap", 2)	
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CC3DMesh;

class CC3DEyeMaterial : public CC3DMaterial
{
public:
	CC3DEyeMaterial();
	virtual ~CC3DEyeMaterial();

	///void  InitMaterial(uint32 MaterialIndex, std::vector<CC3DTexture*>&ModelTexture);

	//void CreateDefault();

	virtual void InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig) override;

	virtual void RenderSet(CC3DMesh* pMesh) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TextureRHI) override;
	virtual void SetParams(const std::string& Params) override;
	//virtual void SetTexture2D(const char* str, GLuint id) override;
	//virtual void SetCubeMap(const char* str, GLuint id) override;
public:

	Vector3 specoffset1;
	Vector3 specoffset2;
	Vector3 specoffset3;
	Vector3 initoffset;
	Vector3 intensity;

	float movespeed1 = 0.5;
	float movespeed2 = 0.5;
	float movespeed3 = 0.5;
	float eyeheight = 0.5;

	std::shared_ptr<CC3DTextureRHI> specularMap;
	std::shared_ptr<CC3DTextureRHI> heightMap;

	DECLARE_SHADER_STRUCT_MEMBER(EyeDoll)
};

#endif // _H_CC3D_TEXTURE_H_
