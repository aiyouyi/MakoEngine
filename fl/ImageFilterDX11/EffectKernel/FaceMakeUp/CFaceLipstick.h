#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/MouthManage.h"
#include "EffectKernel/DXBasicSample.h"
#include "Toolbox/inc.h"

class DX11Texture;

class CFaceLipstick : public CEffectPart, protected DXBaicSample
{
public:
	CFaceLipstick();
	virtual ~CFaceLipstick();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode& childNode, const std::string& path);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	Vector4 m_MouthRGBA = Vector4(0.458824f, 0.050980f, 0.023529f, 0.0);
	float m_wihteTeethAlpha = 0.0f;
	float m_saturation = 0.2f;
	bool isWhitenTeeth = false;
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);
private:
	MouthManage *m_mouthManage;
	DX11Texture *m_MouthMask;

	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
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

	DX11Texture *pTextureBlendWeight = 0;
	DX11Texture *m_material = 0;
};

