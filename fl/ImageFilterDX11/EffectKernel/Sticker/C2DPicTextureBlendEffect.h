#pragma once

#include "EffectKernel/CEffectPart.h"
#include "math\Mat4.h"
#include <xnamath.h>
#include <vector>
#include "Toolbox\EffectModel.hpp"

class cocos2d::Mat4;
class AnimationDrawable;

struct PicTextureBlendConstantBuffer {
	cocos2d::Mat4 mWVP;
	XMVECTOR mColor;
};


class MAGICSTUDIO_EXPORTS_CLASS C2DPicTextureBlendEffect : public CEffectPart {
public:
	C2DPicTextureBlendEffect();
	virtual ~C2DPicTextureBlendEffect();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool WriteConfig(XMLNode& root, HZIP dst, HZIP src = 0) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

	Effect2DRect& getEffect2DRectVec() {
		return m_v2DEffectModel;
	}

	void resetAnidrawable(const std::vector<std::string> & file_list);

	static C2DPicTextureBlendEffect* createEffect();

private:
	AnimationDrawable* readAnidrawable(XMLNode& childNode, HZIP hZip, char* pFilePath);

private:
	Effect2DRect		 m_v2DEffectModel;

	DX11Shader*			 m_StickerShader;
	ID3D11Buffer*		 m_pConstantBuffer;

	__int64				 m_nStartTime;
};