#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/MouthManage.h"
#include "Toolbox/inc.h"
#include <memory>

class CFaceLipstickGL : public CEffectPart
{
public:
	CFaceLipstickGL();
	virtual ~CFaceLipstickGL();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
	Vector4 m_MouthRGBA = Vector4(0.458824f, 0.050980f, 0.023529f, 1.0f);
	float m_wihteTeethAlpha = 0.0f;
private:
	MouthManage *m_mouthManage;
	std::shared_ptr<CC3DTextureRHI> m_MouthMask;

	/// @brief    The gloss contrast scale.
	float m_gloss_contrast_scale = 0.0f;
	/// @brief    The gloss contrast shift.
	float m_gloss_contrast_shift = 0.5f;
	/// @brief    The gloss contrast shrink.
	float m_gloss_contrast_shrink = 1.f;
	/// @brief    The force bright threshold.
	float m_force_bright_threshold = 1.0f;
	float m_gloss_alpha = 0.76f;
	/// @brief    金属光透明度
	float m_metallight_alpha = 1.0f;

	Vector4 m_metalColor = Vector4(1.f, 1.f, 1.f, 1.f);;


	GLuint pTextureBlendWeight = 0;
	GLuint pTextureLevelMap = 0;
	GLuint m_material;
	std::shared_ptr<class CCProgram> m_pShader;
};

