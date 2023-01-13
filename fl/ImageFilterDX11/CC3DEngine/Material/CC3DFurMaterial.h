#ifndef _CC_FUR_MATERIAL_H_
#define _CC_FUR_MATERIAL_H_

#include "CC3DPbrMaterial.h"
#include "FurDataDefine.h"

class CC3DMesh;
class CC3DFurMaterial : public CC3DPBRMaterial
{
public:
	CC3DFurMaterial();
	virtual ~CC3DFurMaterial();


	virtual void InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig) override;
	virtual void PreRenderSet(CC3DMesh* pMesh) override;
	virtual void RenderSet(CC3DMesh* pMesh) override;
	virtual void UpdateModelConfig(CC3DImageFilter::EffectConfig* EffectConfig) override;

	DECLARE_SHADER_STRUCT_MEMBER(FurConstBuffer);
public:
	std::string noise_tex_file;
	std::string default_tex_file;

	std::shared_ptr<MaterialTexRHI> colorTexture = nullptr;
	std::shared_ptr<MaterialTexRHI> noiseTexture = nullptr;
	std::shared_ptr<MaterialTexRHI> lengthTexture = nullptr;

private:
	int numLayers = 30;
	float furLength = 0.1f;
	float UVScale = 20.0f;

};

#endif