#ifndef FacialAnimationInterface_h
#define FacialAnimationInterface_h

#include "common.h"
#include "DX11ImageFilterDef.h"
#include "FaceDetectorInterface.h"
#include <d3d11.h>

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccFacialAnimationCreate();

DX11IMAGEFILTER_EXPORTS_API bool ccFacialAnimationProcessTextureExt(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height, const ccFDFaceRes *faceRes);

//DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationUpdateAvantarParam(cc_handle_t handle, float *arr);

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationSetPath(cc_handle_t handle, const char * path);


DX11IMAGEFILTER_EXPORTS_API bool ccFacialAnimationLoadScene(cc_handle_t handle, const char *szPath);

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationSetBackGround(cc_handle_t handle, const char *szZipFile, const char *szImg);

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationSetCameraArea(cc_handle_t handle, float x, float y, float w, float h);

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationPlay(cc_handle_t handle, const char *szPath, ActionUserCallback pCallBack, void *pUserInfo);

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationDestory(cc_handle_t handle);

DX11IMAGEFILTER_EXPORTS_API bool ccFacialAnimationGetLight(cc_handle_t handle);
#endif