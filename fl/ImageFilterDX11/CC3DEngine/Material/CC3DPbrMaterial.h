#ifndef _CC_PBR_MATERIAL_H_
#define _CC_PBR_MATERIAL_H_

#include "CC3DMaterial.h"

class CC3DMakeUp;

class CC3DPBRMaterial : public CC3DMaterial
{
public:
	CC3DPBRMaterial();
	virtual ~CC3DPBRMaterial();

	virtual void  InitMaterial(uint32 MaterialIndex, const std::vector<std::shared_ptr<CC3DTextureRHI>>& ModelTexture) override;
	virtual void InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig) override;

	virtual void RenderSet(CC3DMesh* pMesh) override;
	virtual void RenderHSV(const std::unordered_map<std::string, Vector4>& HSVColors) override;

	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TextureRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI) override;
	virtual void SetParams(const std::string& Params) override;
	virtual void UpdateModelConfig(CC3DImageFilter::EffectConfig* Config) override;

private:
	void SetFlattenNormal(bool Enable);
	void SetEnableKajiya(bool Enable);

public:
	DECLARE_SHADER_STRUCT_MEMBER(PBRTexture);
	DECLARE_SHADER_STRUCT_MEMBER(PBRToneMapping);
	DECLARE_SHADER_STRUCT_MEMBER(PBRConstantBuffer1);

	bool EnableFN = false;
	Vector4 HSVColor = { 0,0,0,0 };
	std::shared_ptr< CC3DMakeUp> MakeUp;
	bool InitMakeUp = false;
	bool EnableChangeColor = false;
	std::shared_ptr<CC3DTextureRHI> SkinSpecularBRDF;
	std::shared_ptr<CC3DTextureRHI> PreSkinLut;
	std::shared_ptr<CC3DTextureRHI> BlurNormalMap;
	std::shared_ptr<CC3DTextureRHI> shiftMap;
	std::shared_ptr<CC3DTextureRHI> OutlineMask;
	std::shared_ptr<CC3DTextureRHI> EmissiveMaskTexture;
};

#endif