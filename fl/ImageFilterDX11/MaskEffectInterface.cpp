#include "MaskEffectInterface.h"
#include "EffectKernel2/MaskEffect.h"
DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccMaskEffectCreate()
{
	return (cc_handle_t)(new MaskEffect());
}

DX11IMAGEFILTER_EXPORTS_API bool ccMaskEffectProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pCameraTexture, ID3D11ShaderResourceView *pMVTexture, ID3D11Texture2D *pDestTexture, int w, int h, void *pMaskInfo)
{
	MaskEffect *pAction = (MaskEffect *)handle;
	if (pAction != NULL)
	{
		return pAction->renderEffectToTexture(pMVTexture, pCameraTexture, pDestTexture, w, h, pMaskInfo);
	}

	return false;
}

DX11IMAGEFILTER_EXPORTS_API bool ccMaskEffectGenRGBATexture(cc_handle_t handle, ID3D11ShaderResourceView *pCameraTexture, ID3D11Texture2D *pDestTexture, int w, int h, void *pMaskInfo)
{
	MaskEffect *pAction = (MaskEffect *)handle;
	if (pAction != NULL && pMaskInfo != NULL)
	{
		return pAction->renderRGBATexture(pCameraTexture, pDestTexture, w, h, pMaskInfo);
	}

	return false;
}

DX11IMAGEFILTER_EXPORTS_API void ccMaskEffectDestory(cc_handle_t handle)
{
	MaskEffect *pAction = (MaskEffect *)handle;
	if (pAction != NULL)
	{
		pAction->destory();
		delete pAction;
	}
}