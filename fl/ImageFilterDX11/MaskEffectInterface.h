#ifndef MaskEffectInterface_h
#define MaskEffectInterface_h

#include "common.h"
#include "DX11ImageFilterDef.h"
#include <d3d11.h>
DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccMaskEffectCreate();
DX11IMAGEFILTER_EXPORTS_API bool ccMaskEffectProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pCameraTexture, ID3D11ShaderResourceView *pMVTexture, ID3D11Texture2D *pDestTexture, int w, int h, void *pMaskInfo);
DX11IMAGEFILTER_EXPORTS_API bool ccMaskEffectGenRGBATexture(cc_handle_t handle, ID3D11ShaderResourceView *pCameraTexture, ID3D11Texture2D *pDestTexture, int w, int h, void *pMaskInfo);
DX11IMAGEFILTER_EXPORTS_API void ccMaskEffectDestory(cc_handle_t handle);
#endif