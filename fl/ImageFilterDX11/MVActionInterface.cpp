#include "MVActionInterface.h"
#include "EffectKernel2/MVAction.h"
DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccMVActionCreate()
{
	return (cc_handle_t)(new MVAction());
}

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionLoad(cc_handle_t handle, const char *szAction)
{
	MVAction *pAction = (MVAction *)handle;
	if (pAction != NULL)
	{
		return pAction->loadAction(szAction);
	}

	return false;
}

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height)
{
	MVAction *pAction = (MVAction *)handle;
	if (pAction != NULL)
	{
		return pAction->renderEffectToTexture(pTexture, pDestTexture, width, height);
	}
	return false;
}

DX11IMAGEFILTER_EXPORTS_API void ccMVActionReset(cc_handle_t handle)
{
	MVAction *pAction = (MVAction *)handle;
	if (pAction != NULL)
	{
		pAction->reset();
	}
}

DX11IMAGEFILTER_EXPORTS_API void ccMVActionDestory(cc_handle_t handle)
{
	MVAction *pAction = (MVAction *)handle;
	if (pAction != NULL)
	{
		pAction->destory();
		delete pAction;
	}
}