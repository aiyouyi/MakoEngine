#include "FacialAnimationInterface.h"
#include "EffectKernel2/FacialAnimation.h"

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccFacialAnimationCreate()
{
	return (cc_handle_t)(new FacialAnimation());
}

DX11IMAGEFILTER_EXPORTS_API bool ccFacialAnimationProcessTextureExt(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	return ((FacialAnimation*)handle)->renderEffectToTexture(pTexture, pDestTexture, width, height, faceRes);
}
DX11IMAGEFILTER_EXPORTS_API bool ccFacialAnimationLoadScene(cc_handle_t handle, const char *szPath)
{
	return ((FacialAnimation*)handle)->loadScene(szPath);
}


DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationSetPath(cc_handle_t handle, const char *path)
{
	assert(handle != 0);
	return ((FacialAnimation*)handle)->SetResourcePath(path);
}

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationSetBackGround(cc_handle_t handle, const char *szZipFile, const char *szImg)
{
	((FacialAnimation*)handle)->setBackGround(szZipFile, szImg);
}

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationSetCameraArea(cc_handle_t handle, float x, float y, float w, float h)
{
	((FacialAnimation*)handle)->SetCameraArea(x, y, w, h);
}

// DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationUpdateAvantarParam(cc_handle_t handle, float *arr)
// {
// 	return ((FacialAnimation*)handle)->updateAvantarParam(arr);
// }

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationPlay(cc_handle_t handle, const char *szPath, ActionUserCallback pCallBack, void *pUserInfo)
{
	return ((FacialAnimation*)handle)->play(szPath, pCallBack, pUserInfo);
}

DX11IMAGEFILTER_EXPORTS_API void ccFacialAnimationDestory(cc_handle_t handle)
{
	assert(handle != 0);
	((FacialAnimation*)handle)->destory();
	delete ((FacialAnimation*)handle);
}

bool ccFacialAnimationGetLight(cc_handle_t handle)
{
	return false;
}
