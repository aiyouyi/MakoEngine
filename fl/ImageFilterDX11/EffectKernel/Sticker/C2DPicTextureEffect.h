#pragma once

#include "EffectKernel/CEffectPart.h"
#include "math\Mat4.h"
#include <xnamath.h>
#include <vector>
#include "Toolbox\EffectModel.hpp"

class cocos2d::Mat4;
class AnimationDrawable;

struct PicTextureConstantBuffer {
	cocos2d::Mat4 mWVP;
	XMVECTOR mColor;
};

struct RectInfo 
{
	int x;
	int y;
	int width;
	int height;
};

class MAGICSTUDIO_EXPORTS_CLASS C2DPicTextureEffect : public CEffectPart {
public:
	C2DPicTextureEffect();
	virtual ~C2DPicTextureEffect();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

	void updateData();

public:
	int alignType = EAPT_CC;
	Vector2 m_SrcRect[2];
private:
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL, const std::string &path = "");
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);



	Effect2DRect		 m_v2DEffectModel;

	DX11Shader*			 m_StickerShader = NULL;
	ID3D11Buffer*		 m_pConstantBuffer = NULL;

};