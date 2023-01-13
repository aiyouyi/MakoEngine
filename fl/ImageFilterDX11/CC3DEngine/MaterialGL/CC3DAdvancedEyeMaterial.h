#ifndef _CC_ADVANCED_EYE_MATERIAL_H_
#define _CC_ADVANCED_EYE_MATERIAL_H_

#include "CC3DMaterialGL.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Render/TextureRHI.h"

BEGIN_SHADER_STRUCT(AdvancedEye, 2)
	DECLARE_PARAM(Vector4, _scleraColor)
	DECLARE_PARAM(Vector4, _irisColor)
	DECLARE_PARAM(Vector4, _illumColor)
	DECLARE_PARAM_VALUE(float, _scleraShadowAmt, 0.0)
	DECLARE_PARAM_VALUE(float, _irisShadowAmt, 0.0)
	DECLARE_PARAM_VALUE(float, _irisSize, 1.88)
	DECLARE_PARAM_VALUE(float, _scleraSize, 0.968)
	DECLARE_PARAM_VALUE(float, _limbus, 0.5)
	DECLARE_PARAM_VALUE(float, _parallax, 0.05)
	DECLARE_PARAM_VALUE(float, _smoothness, 0.75)
	DECLARE_PARAM_VALUE(float, _specsize, 0.9)
	DECLARE_PARAM_VALUE(float, _reflectTerm, 0.025)
	BEGIN_STRUCT_CONSTRUCT(AdvancedEye)
		IMPLEMENT_PARAM("_scleraColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("_irisColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("_illumColor", UniformType::FLOAT4)
		IMPLEMENT_PARAM("_scleraShadowAmt", UniformType::FLOAT)
		IMPLEMENT_PARAM("_irisShadowAmt", UniformType::FLOAT)
		IMPLEMENT_PARAM("_irisSize", UniformType::FLOAT)
		IMPLEMENT_PARAM("_scleraSize", UniformType::FLOAT)
		IMPLEMENT_PARAM("_limbus", UniformType::FLOAT)
		IMPLEMENT_PARAM("_parallax", UniformType::FLOAT)
		IMPLEMENT_PARAM("_smoothness", UniformType::FLOAT)
		IMPLEMENT_PARAM("_specsize", UniformType::FLOAT)
		IMPLEMENT_PARAM("_reflectTerm", UniformType::FLOAT)
		INIT_TEXTURE_INDEX("_IrisColorTex", 0)
		INIT_TEXTURE_INDEX("_IrisTex", 1)
		INIT_TEXTURE_INDEX("_MainTex", 2)
		INIT_TEXTURE_INDEX("_ShadeScleraTex", 3)
		INIT_TEXTURE_INDEX("_ShadeIrisTex", 4)
		INIT_TEXTURE_INDEX("_CorneaBump", 5)
		INIT_TEXTURE_INDEX("_EyeBump", 6)
		INIT_TEXTURE_INDEX("_IrisBump", 7)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CC3DAdvancedEyeMaterial : public CC3DMaterialGL
{
public:
	CC3DAdvancedEyeMaterial();
	virtual ~CC3DAdvancedEyeMaterial();

	virtual void InitShaderProgram(std::string path) override;

	virtual void RenderSet(CC3DMesh* pMesh) override;

	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI) override;
	virtual void SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI) override;
	virtual void SetParams(const std::string& Params) override;

	virtual void LoadConfig(const std::string config_file);
public:
	std::shared_ptr<CC3DTextureRHI> irisColorTex;
	std::shared_ptr<CC3DTextureRHI> irisTex;
	std::shared_ptr<CC3DTextureRHI> mainTex;
	std::shared_ptr<CC3DTextureRHI> shadeScleraTex;
	std::shared_ptr<CC3DTextureRHI> shadeIrisTex;
	std::shared_ptr<CC3DTextureRHI> corneaBump;
	std::shared_ptr<CC3DTextureRHI> eyeBump;
	std::shared_ptr<CC3DTextureRHI> irisBump;

	float _scleraShadowAmt = 0.0;
	float _irisShadowAmt = 0.0;
	float _irisSize = 1.88f;
	float _scleraSize = 0.968f;
	float _limbus = 0.5f;
	Vector4 _scleraColor = (0.95, 0.95, 0.95, 1);
	Vector4 _irisColor = (1, 1, 1, 1);
	Vector4 _illumColor = (0, 0, 0, 0);
	float _parallax = 0.05f;
	float _smoothness = 0.75f;
	float _specsize = 0.9f;
	float _reflectTerm = 0.025f;

	DECLARE_SHADER_STRUCT_MEMBER(AdvancedEye)
};

#endif