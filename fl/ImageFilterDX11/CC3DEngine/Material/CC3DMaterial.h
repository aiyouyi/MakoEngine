#pragma once
#ifndef _H_CC3D_MATERIAL_H_
#define _H_CC3D_MATERIAL_H_

#include <unordered_map>
#include <map>
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "CC3DPbrMaterialDef.h"

#define LIGHT_NUM 4

enum class MaterialType : uint8_t
{
	PBR,
	FUR
};


class CC3DMesh;

namespace CC3DImageFilter
{
	struct EffectConfig;
}

class CC3DMaterial : public CCglTFModel
{
public:
	CC3DMaterial();
	virtual ~CC3DMaterial();

	void CreateDefault();

	virtual void  InitMaterial(uint32 MaterialIndex, const std::vector<std::shared_ptr<CC3DTextureRHI>> &ModelTexture);
	virtual void InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig) = 0;
	virtual void UseShader();
	virtual void PreRenderSet(CC3DMesh* pMesh);
	virtual void RenderSet(CC3DMesh* pMesh);
	virtual void SetParams(const std::string& Params) {};
	virtual void UpdateModelConfig(CC3DImageFilter::EffectConfig* Config) {};
	virtual void RenderHSV(const std::unordered_map<std::string, Vector4>& HSVColors) {};

	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TextureRHI);
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI);
	virtual void DrawTriangle(CC3DMesh* pMesh);


	template<typename T>
	void SetParameter(const std::string& name, const T& value)
	{
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter(name, value);
	}
	template<typename T>
	void DebugSetParameter(const std::string& name, const T& value)
	{
		auto pos = GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).InfoMap_.find(name); 
		if (pos != GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).InfoMap_.end())
		{
			size_t offset = pos->second.offset;
			void* common_ptr = &GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ConstBuffer;
			void* p = static_cast<char*>(common_ptr) + offset;
			memcpy(p, &value, sizeof(value));
		}
	}

	template<typename T,int Num>
	void SetParameter(const std::string& name, const T(&value)[Num])
	{
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter<T, Num>(name, value);
	}

	void SetBoneMatrix(const glm::mat4& mat, int index);
	

	std::shared_ptr<CC3DTextureRHI> m_BaseColorTexture;
	std::shared_ptr<CC3DTextureRHI> m_MetallicRoughnessTexture;
	std::shared_ptr<CC3DTextureRHI> m_NormalTexture;
	std::shared_ptr<CC3DTextureRHI> m_EmissiveTexture;
	std::shared_ptr<CC3DTextureRHI> m_OcclusionTexture;
	Vector4 HSVA = Vector4(0, 0, 0, 0);

	std::string MaterialName;
	std::string alphaMode;
	float alphaCutoff;
	bool doubleSided;
	float metallicFactor;
	float roughnessFactor;

	MaterialType materialType = MaterialType::PBR;

	DECLARE_SHADER_STRUCT_MEMBER(PBRConstantBuffer);
	DECLARE_SHADER_STRUCT_MEMBER(PBRSkinMat);
	

	std::shared_ptr<class ShaderRHI> mShader;

};

#endif // _H_CC3D_TEXTURE_H_
