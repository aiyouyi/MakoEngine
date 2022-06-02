#pragma once
#ifndef _H_CC3D_MATERIAL_H_
#define _H_CC3D_MATERIAL_H_

#include <unordered_map>
#include <map>
#include "CC3DTexture.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"

#define LIGHT_NUM 4

enum class MaterialType : uint8_t
{
	PBR,
	FUR
};

BEING_SHADER_STRUCT(PBRConstantBuffer,0)
	DELCARE_PARAM(glm::mat4,world)
	DELCARE_PARAM(glm::mat4,meshMat)
	DELCARE_PARAM(glm::mat4, view)
	DELCARE_PARAM(glm::mat4, projection)
	DELCARE_PARAM(glm::mat4, meshMatInverse)
	DELCARE_PARAM(glm::mat4, RotateIBL)
	DELCARE_PARAM(glm::mat4, lightSpaceMatrix)
	DELCARE_ARRAY_PARAM(Vector4,4, lightDir)
	DELCARE_ARRAY_PARAM(Vector4, 4, lightColors)
	DELCARE_PARAM_VALUE(int, LightNum,0)
	DELCARE_PARAM(Vector3, CamPos)
	DELCARE_PARAM_VALUE(float, ambientStrength, 1.f)
	DELCARE_PARAM_VALUE(float, RoughnessRate, 1.f)
	DELCARE_PARAM_VALUE(int, AnimationEnable, 0)
	DELCARE_PARAM_VALUE(int, ReverseY, 1)
	DELCARE_PARAM_VALUE(int, ShadowsEnable, 0)
	DELCARE_PARAM_VALUE(float, gamma, 1.f)
	DELCARE_PARAM_VALUE(int, u_EnbleRMGamma, 0)
	DELCARE_PARAM_VALUE(int, u_EnbleEmiss,0)
	DELCARE_PARAM_VALUE(float, NormalIntensity, 0.0)
	DELCARE_PARAM_VALUE(float, FrontNoramlScale, 1.0)
	DELCARE_PARAM_VALUE(float, FrontNormalOffset, 0.0)
	DELCARE_PARAM_VALUE(int, EnableKajiya, 0)
	DELCARE_PARAM_VALUE(float, PrimaryShift, 0.2)
	DELCARE_PARAM_VALUE(float, SecondaryShift, 0.2)
	DELCARE_PARAM_VALUE(float, SpecularPower, 100)
	DELCARE_PARAM_VALUE(float, ShiftU, 1.0)
	DELCARE_PARAM_VALUE(float, KajiyaSpecularScale, 0.5)
	DELCARE_PARAM_VALUE(float, KajiyaSpecularWidth, 0.5)
	DELCARE_PARAM_VALUE(int, EnableRenderOutLine, 0)
	DELCARE_PARAM_VALUE(float, OutlineWidth, 0.001)
	DELCARE_PARAM(Vector4, OutLineColor)
	BEING_STRUCT_CONSTRUCT(PBRConstantBuffer)
		IMPLEMENT_PARAM("world", UniformType::MAT4)
		IMPLEMENT_PARAM("meshMat", UniformType::MAT4)
		IMPLEMENT_PARAM("view", UniformType::MAT4)
		IMPLEMENT_PARAM("projection", UniformType::MAT4)
		IMPLEMENT_PARAM("meshMatInverse", UniformType::MAT4)
		IMPLEMENT_PARAM("RotateIBL", UniformType::MAT4)
		IMPLEMENT_PARAM("lightSpaceMatrix", UniformType::MAT4)
		IMPLEMENT_PARAM("lightDir", UniformType::MAT4)
		IMPLEMENT_PARAM("lightColors", UniformType::MAT4)
		IMPLEMENT_PARAM("lightNum", UniformType::INT)
		IMPLEMENT_PARAM("camPos", UniformType::FLOAT3)
		IMPLEMENT_PARAM("ambientStrength", UniformType::FLOAT)
		IMPLEMENT_PARAM("roughnessRate", UniformType::FLOAT)
		IMPLEMENT_PARAM("AnimationEnable", UniformType::INT)
		IMPLEMENT_PARAM("reverseY", UniformType::INT)
		IMPLEMENT_PARAM("ShadowsEnable", UniformType::INT)
		IMPLEMENT_PARAM("gamma", UniformType::FLOAT)
		IMPLEMENT_PARAM("u_EnbleRMGamma", UniformType::INT)
		IMPLEMENT_PARAM("u_EnbleEmiss", UniformType::INT)
		IMPLEMENT_PARAM("NormalIntensity", UniformType::FLOAT)
		IMPLEMENT_PARAM("FrontNoramlScale", UniformType::FLOAT)
		IMPLEMENT_PARAM("FrontNormalOffset", UniformType::FLOAT)
		IMPLEMENT_PARAM("EnableKajiya", UniformType::INT)
		IMPLEMENT_PARAM("PrimaryShift", UniformType::FLOAT)
		IMPLEMENT_PARAM("SecondaryShift", UniformType::FLOAT)
		IMPLEMENT_PARAM("SpecularPower", UniformType::FLOAT)
		IMPLEMENT_PARAM("ShiftU", UniformType::FLOAT)
		IMPLEMENT_PARAM("KajiyaSpecularScale", UniformType::FLOAT)
		IMPLEMENT_PARAM("KajiyaSpecularWidth", UniformType::FLOAT)
		IMPLEMENT_PARAM("EnableRenderOutLine", UniformType::INT)
		IMPLEMENT_PARAM("OutlineWidth", UniformType::FLOAT)
		IMPLEMENT_PARAM("OutLineColor", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

BEING_SHADER_STRUCT(PBRSkinMat,1)
	DELCARE_ARRAY_PARAM(glm::mat4, MAX_MATRICES, BoneMat)
	BEING_STRUCT_CONSTRUCT(PBRSkinMat)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CC3DMesh;
class DX11Shader;

class CC3DMaterial : public CCglTFModel
{
public:
	CC3DMaterial();
	virtual ~CC3DMaterial();


	void  InitMaterial(uint32 MaterialIndex, std::vector<std::shared_ptr<CC3DTextureRHI>> &ModelTexture);
	void CreateDefault();

	virtual void InitShaderProgram(std::string path);
	virtual void UseShader();
	virtual void PreRenderSet(CC3DMesh* pMesh);
	virtual void RenderSet(CC3DMesh* pMesh);
	virtual void SetParams(const std::string& Params) {};

	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI);
	virtual void DrawTriangle(CC3DMesh* pMesh);

	//TODO:还有数组类型的uniform没有解决，好像可以直接使用size去包含整个数组大小
	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter(name, value, size);
	}

	void SetBoneMatrix(glm::mat4 mat, int index);


	std::shared_ptr<CC3DTextureRHI> m_BaseColorTexture;
	std::shared_ptr<CC3DTextureRHI> m_MetallicRoughnessTexture;
	std::shared_ptr<CC3DTextureRHI> m_NormalTexture;
	std::shared_ptr<CC3DTextureRHI> m_EmissiveTexture;
	std::shared_ptr<CC3DTextureRHI> m_OcclusionTexture;
	Vector4 HSVA = Vector4(0, 0, 0, 0);

	std::vector<std::shared_ptr<CC3DTextureRHI>>m_ModelTexture;

	std::string MaterialName;
	std::string alphaMode;
	float alphaCutoff;
	bool doubleSided;
	float metallicFactor;
	float roughnessFactor;

	MaterialType materialType = MaterialType::PBR;


	DELCARE_SHADER_STRUCT_MEMBER(PBRConstantBuffer);
	DELCARE_SHADER_STRUCT_MEMBER(PBRSkinMat);

	DX11Shader* pShader = nullptr;

};

#endif // _H_CC3D_TEXTURE_H_
