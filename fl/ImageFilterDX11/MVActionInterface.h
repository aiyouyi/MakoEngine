#ifndef MVActionInterface_h
#define MVActionInterface_h

#include "common.h"
#include "DX11ImageFilterDef.h"
#include <d3d11.h>

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccMVActionCreate();

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionLoad(cc_handle_t handle, const char *szAction);

DX11IMAGEFILTER_EXPORTS_API bool ccMVActionProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height);

DX11IMAGEFILTER_EXPORTS_API void ccMVActionReset(cc_handle_t handle);

DX11IMAGEFILTER_EXPORTS_API void ccMVActionDestory(cc_handle_t handle);
#endif