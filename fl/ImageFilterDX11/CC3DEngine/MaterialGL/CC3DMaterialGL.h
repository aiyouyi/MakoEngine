#pragma once
#ifndef _H_CC3D_TEXTURE_H_
#define _H_CC3D_TEXTURE_H_
#include <unordered_map>
#include "Material/CC3DTexture.h"

#include "ToolBox/GL/CCProgram.h"
#include "ToolBox/Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Material/CC3DPbrMaterialDef.h"
#define LIGHT_NUM 4

enum class MaterialTypeGL : uint8_t
{
	PBR,
	FUR,
	EYE,
	DYNAMICPBR,
	BLINNPHONG,
	DYFUR
};



class CC3DMesh;

class CC3DMaterialGL : public CCglTFModel
{
public:
	CC3DMaterialGL();
	virtual ~CC3DMaterialGL();

	void  InitMaterial(uint32 MaterialIndex,const std::vector<std::shared_ptr<CC3DTextureRHI>>&ModelTexture);

	void CreateDefault();

	virtual void InitShaderProgram(std::string path);

	virtual void UseProgram();
	virtual void UseShader();

	virtual void PreRenderSet(CC3DMesh* pMesh);
	virtual void RenderSet(CC3DMesh* pMesh);
	virtual void SetParams(const std::string& Params);
	virtual void RenderHSV(const std::unordered_map<std::string, Vector4>& HSVColors) {};
	virtual void DrawTriangle(CC3DMesh* pMesh);

	virtual void SetUniformMatrix4fv(const char *str, const GLfloat *array, bool transpose, int count);
	virtual void SetUniform3f(const char * str, float x, float y, float z);
	virtual void SetUniform4f(const char* str, float x, float y, float z, float w);
	virtual void SetUniform1f(const char* str, float value);
	virtual void SetUniform1i(const char* str, int value);
	virtual void SetTexture2D(const char* str, GLuint id);
	virtual void SetCubeMap(const char* str, GLuint id);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TexRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TexRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> TexRHI);
	virtual void SetCubeMap(const char* str, std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI);

	virtual void LoadConfig(const std::string config_file);
	virtual void LoadEnvironmentConfig() {}

	template<typename T>
	void SetParameter(std::string name, const T& value)
	{
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter(name, value);
	}

	template<typename T, int Num>
	void SetParameter(std::string name, const T(&value)[Num])
	{
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter<T, Num>(name, value);
	}

	void SetBoneMatrix(const glm::mat4& mat, int index);

public:
	std::shared_ptr<CC3DTextureRHI> m_BaseColorTexture;
	std::shared_ptr<CC3DTextureRHI> m_MetallicRoughnessTexture;
	std::shared_ptr<CC3DTextureRHI> m_NormalTexture;
	std::shared_ptr<CC3DTextureRHI> m_EmissiveTexture;
	std::shared_ptr<CC3DTextureRHI> m_OcclusionTexture;
	float base_emiss_factor = 0.0f;
	float extra_emiss_factor = 0.0f;


	std::string MaterialName;
	std::string alphaMode;
	float alphaCutoff;                  
	bool doubleSided;                   
	float metallicFactor;   
	float roughnessFactor;  

	MaterialTypeGL materialType = MaterialTypeGL::PBR;

	CCProgram* pShader = nullptr;
	unsigned int texture_index;

	std::unordered_map<std::string, int> texture_map;

	DECLARE_SHADER_STRUCT_MEMBER(PBRConstantBuffer);
	DECLARE_SHADER_STRUCT_MEMBER(PBRSkinMat);
	std::shared_ptr<class ShaderRHI> mShader;
};

#endif // _H_CC3D_TEXTURE_H_
