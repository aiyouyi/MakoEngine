#include "MaskEffect.h"
#include "Toolbox/Helper.h"
#include <fstream>  
#include <string>
using namespace std;

MaskEffect::MaskEffect()
{
	m_pFBO = NULL;
	m_rectDraw = NULL;

	m_pTargetTexture = NULL;
}

MaskEffect::~MaskEffect()
{
	destory();
}

bool MaskEffect::renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11ShaderResourceView *pVideoTexture, ID3D11Texture2D *pTargetTexture, int width, int height, void *pMaskInfo)
{
	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height || m_pTargetTexture != pTargetTexture)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, true, pTargetTexture);

		m_pTargetTexture = pTargetTexture;
	}
	if (m_rectDraw == NULL)
	{
		m_rectDraw = new RectDraw();
		m_rectDraw->init(1, 1);
	}

	m_pFBO->bind();
	float arrClip[] = { 0,0,1,1 };

	struct SMaskInfo
	{
		unsigned char type;
		unsigned char *pMask;
		int maskW;
		int maskH;
	};
	SMaskInfo *info = (SMaskInfo *)pMaskInfo;
	m_rectDraw->setShaderTextureView(pInputTexture);
	m_rectDraw->render(pVideoTexture, arrClip, info->pMask, info->maskW, info->maskH);
	return true;
}

bool MaskEffect::renderRGBATexture(ID3D11ShaderResourceView *pVideoTexture, ID3D11Texture2D *pTargetTexture, int width, int height, void *pMaskInfo)
{
	if (m_pFBO == NULL || m_pFBO->width() != width || m_pFBO->height() != height || m_pTargetTexture != pTargetTexture)
	{
		if (m_pFBO != NULL)
		{
			delete m_pFBO;
		}

		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(width, height, true, pTargetTexture);

		m_pTargetTexture = pTargetTexture;
	}
	if (m_rectDraw == NULL)
	{
		m_rectDraw = new RectDraw();
		m_rectDraw->init(1, 1);
	}

	m_pFBO->bind();
	m_pFBO->clear(0, 0, 0, 0);
	float arrClip[] = { 0,0,1,1 };

	struct SMaskInfo
	{
		unsigned char type;
		unsigned char *pMask;
		int maskW;
		int maskH;
	};
	SMaskInfo *info = (SMaskInfo *)pMaskInfo;
	m_rectDraw->setShaderTextureView(pVideoTexture);
	m_rectDraw->renderAlpha(arrClip, info->pMask, info->maskW, info->maskH, width, height);
	return true;
}

void MaskEffect::destory()
{
	SAFEDEL(m_pFBO);
	SAFEDEL(m_rectDraw);

	m_pTargetTexture = NULL;
}