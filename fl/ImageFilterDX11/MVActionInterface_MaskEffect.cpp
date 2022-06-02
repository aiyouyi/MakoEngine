#include "MVActionInterface.h"
#include "MaskEffectInterface.h"
#include "EffectKernel2/MVAction.h"
#include "EffectKernel2/MaskEffect.h"

struct MV_MaskEffect
{
public:
	MV_MaskEffect()
	{
		h_maskeffect = 0;
		h_mvAction = 0;

		m_texW = 0;
		m_texH = 0;

		m_renderTargetTex = NULL;
	}

	MaskEffect *h_maskeffect;
	MVAction *h_mvAction;
	DX11Texture *m_renderTargetTex;
	int m_texW;
	int m_texH;
};

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccMVActionCreate_MaskEffect()
{
	MV_MaskEffect *mv_maskEffect = new MV_MaskEffect();
	mv_maskEffect->h_maskeffect = (MaskEffect *)ccMaskEffectCreate();
	mv_maskEffect->h_mvAction = (MVAction *)ccMVActionCreate();

	return (cc_handle_t)(mv_maskEffect);
}

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionLoad_MaskEffect(cc_handle_t handle, const char *szAction)
{
	MV_MaskEffect *mv_maskEffect = (MV_MaskEffect *)(handle);
	if (mv_maskEffect != NULL && mv_maskEffect->h_mvAction != NULL)
	{
		return mv_maskEffect->h_mvAction->loadAction(szAction);
	}

	return false;
}

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionProcessTexture_MaskEffect(cc_handle_t handle, ID3D11ShaderResourceView *pCameraTexture, ID3D11ShaderResourceView *pMVTexture, ID3D11Texture2D *pDestTexture, int width, int height, void *pMaskInfo)
{
	MV_MaskEffect *mv_maskEffect = (MV_MaskEffect *)(handle);
	if (mv_maskEffect != NULL && mv_maskEffect->h_maskeffect != NULL && mv_maskEffect->h_mvAction != NULL)
	{
		if (mv_maskEffect->m_texH != height || mv_maskEffect->m_texW != width)
		{
			if (mv_maskEffect->m_renderTargetTex != NULL)
			{
				mv_maskEffect->m_renderTargetTex->destory();
				delete mv_maskEffect->m_renderTargetTex;
			}

			mv_maskEffect->m_renderTargetTex = new DX11Texture();
			mv_maskEffect->m_renderTargetTex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, width, height);

			mv_maskEffect->m_texW = width;
			mv_maskEffect->m_texH = height;
		}

		mv_maskEffect->h_maskeffect->renderEffectToTexture(pCameraTexture, pMVTexture, mv_maskEffect->m_renderTargetTex->getTex(), width, height, pMaskInfo);
		return mv_maskEffect->h_mvAction->renderEffectToTexture(mv_maskEffect->m_renderTargetTex->getTexShaderView(), pDestTexture, width, height);
	}
	return false;
}

DX11IMAGEFILTER_EXPORTS_API void ccMVActionReset_MaskEffect(cc_handle_t handle)
{
	MV_MaskEffect *mv_maskEffect = (MV_MaskEffect *)(handle);
	if (mv_maskEffect != NULL && mv_maskEffect->h_mvAction != NULL)
	{
		return mv_maskEffect->h_mvAction->reset();
	}
}

DX11IMAGEFILTER_EXPORTS_API void ccMVActionDestory_MaskEffect(cc_handle_t handle)
{
	MV_MaskEffect *mv_maskEffect = (MV_MaskEffect *)(handle);
	if (mv_maskEffect != NULL)
	{
		if (mv_maskEffect->h_maskeffect != NULL)
		{
			mv_maskEffect->h_maskeffect->destory();
			delete mv_maskEffect->h_maskeffect;
			mv_maskEffect->h_maskeffect = 0;
		}

		if (mv_maskEffect->h_mvAction != NULL)
		{
			mv_maskEffect->h_mvAction->destory();
			delete mv_maskEffect->h_mvAction;
			mv_maskEffect->h_mvAction = 0;
		}

		if (mv_maskEffect->m_renderTargetTex != NULL)
		{
			mv_maskEffect->m_renderTargetTex->destory();
			delete mv_maskEffect->m_renderTargetTex;
			mv_maskEffect->m_renderTargetTex = 0;
		}
		delete mv_maskEffect;
	}
}