#ifndef MVActionInterface_MaskEffect_h
#define MVActionInterface_MaskEffect_h

#include "common.h"
#include "DX11ImageFilterDef.h"
#include <d3d11.h>

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccMVActionCreate_MaskEffect();

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionLoad_MaskEffect(cc_handle_t handle, const char *szAction);

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionProcessTexture_MaskEffect(cc_handle_t handle, ID3D11ShaderResourceView *pCameraTexture, ID3D11ShaderResourceView *pMVTexture, ID3D11Texture2D *pDestTexture, int w, int h, void *pMaskInfo);

DX11IMAGEFILTER_EXPORTS_API void ccMVActionReset_MaskEffect(cc_handle_t handle);

DX11IMAGEFILTER_EXPORTS_API void ccMVActionDestory_MaskEffect(cc_handle_t handle);
#endif