#ifndef StickerInterface_h
#define StickerInterface_h

#include "common.h"
#include "DX11ImageFilterDef.h"
#include "FaceDetectorInterface.h"
#include <d3d11.h>

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccStickerCreate();

DX11IMAGEFILTER_EXPORTS_API ID3D11ShaderResourceView *ccStickerProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes);

DX11IMAGEFILTER_EXPORTS_API bool ccStickerProcessTextureExt(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo);

//arrRenderView[6]:ÌùÖ½±³¾°x,y,width,height rendertarget width£¬height
DX11IMAGEFILTER_EXPORTS_API void ccStickerProcess(cc_handle_t handle, int width, int height, const ccFDFaceRes *faceRes, int *arrRenderView = NULL);

DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffect(cc_handle_t handle, const char *szPath, const char *szFile, EffectCallback callback, void *lpParam);
//DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffectZip(cc_handle_t handle, const char *szZipPath, const char *szXML);
//DX11IMAGEFILTER_EXPORTS_API bool ccStickerSetEffectZip_aysn(cc_handle_t handle, const char *szZipPath, const char *szXML);
DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffectZip(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam);
DX11IMAGEFILTER_EXPORTS_API bool ccStickerSetEffectZip_aysn(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam);

DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffectCacheSize(cc_handle_t handle, int nSize);

DX11IMAGEFILTER_EXPORTS_API void ccStickerResetAnimation(cc_handle_t handle);

DX11IMAGEFILTER_EXPORTS_API void ccStickerDestory(cc_handle_t handle);
#endif