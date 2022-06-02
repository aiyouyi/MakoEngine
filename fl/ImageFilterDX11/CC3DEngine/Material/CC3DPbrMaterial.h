#ifndef _CC_PBR_MATERIAL_H_
#define _CC_PBR_MATERIAL_H_

#include "CC3DMaterial.h"

BEING_SHADER_STRUCT(PBRTexture,-1)
BEING_STRUCT_CONSTRUCT(PBRTexture)
INIT_TEXTURE_INDEX("irradianceMap", 4)
INIT_TEXTURE_INDEX("prefilterMap", 5)
INIT_TEXTURE_INDEX("brdfLUT", 6)
INIT_TEXTURE_INDEX("ShadowMap", 7)
INIT_TEXTURE_INDEX("ShiftTex", 9)
INIT_TEXTURE_INDEX("OutLineMask", 10)
END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CC3DPBRMaterial : public CC3DMaterial
{
public:
	CC3DPBRMaterial();
	virtual ~CC3DPBRMaterial();

	virtual void InitShaderProgram(std::string path) override;

	virtual void RenderSet(CC3DMesh* pMesh) override;


	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI) override;
	virtual void SetParams(const std::string& Params) override;

	void SetFlattenNormal(bool Enable);
	void SetEnableKajiya(bool Enable);

private:
	DELCARE_SHADER_STRUCT_MEMBER(PBRTexture);
	bool EnableFN = false;
};

#endif