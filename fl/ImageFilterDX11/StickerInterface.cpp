#include "StickerInterface.h"
#include "EffectKernel2/StickerEffect.h"
#include <assert.h>

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccStickerCreate()
{
	return (cc_handle_t)(new StickerEffect());
}

DX11IMAGEFILTER_EXPORTS_API ID3D11ShaderResourceView *ccStickerProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	assert(handle != 0);
	//if (faceRes == 0)
	//{
		//return pTexture;
	//}

	return ((StickerEffect*)handle)->renderEffectToTexture(pTexture, width, height, faceRes);
}

DX11IMAGEFILTER_EXPORTS_API bool ccStickerProcessTextureExt(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo)
{
	assert(handle != 0);
	//if (faceRes == 0)
	//{
		//return false;
	//}

	return ((StickerEffect*)handle)->renderEffectToTexture(pTexture, pDestTexture, width, height, faceRes, pExtInfo);
	//return true;
}

DX11IMAGEFILTER_EXPORTS_API void ccStickerProcess(cc_handle_t handle, int width, int height, const ccFDFaceRes *faceRes, int *arrRenderView)
{
	if (faceRes == 0)return;

	assert(handle != 0);
	((StickerEffect*)handle)->renderEffect(width, height, faceRes, arrRenderView);
}

DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffect(cc_handle_t handle, const char *szPath, const char *szFile, EffectCallback callback, void *lpParam)
{
	assert(handle != 0);

	string szInputFile;
	if (szFile == NULL)
	{
		szInputFile = "";
	}
	else
	{
		szInputFile = szFile;
	}

	string szInputPath;
	if (szPath == NULL)
	{
		szInputPath = "";
	}
	else
	{
		szInputPath = szPath;
	}
	((StickerEffect*)handle)->loadEffect(szInputPath, szInputFile, callback, lpParam);
}
/*
DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffectZip(cc_handle_t handle, const char *szZipPath, const char *szXML)
{
	ccStickerSetEffectZip(handle, szZipPath, szXML, NULL, NULL);
}

DX11IMAGEFILTER_EXPORTS_API bool ccStickerSetEffectZip_aysn(cc_handle_t handle, const char *szZipPath, const char *szXML)
{
	ccStickerSetEffectZip_aysn(handle, szZipPath, szXML, NULL, NULL);
}
*/

DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffectZip(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam)
{
	assert(handle != 0);

	string szInputFile;
	if (szXML == NULL)
	{
		szInputFile = "";
	}
	else
	{
		szInputFile = szXML;
	}

	string szInputPath;
	if (szZipPath == NULL)
	{
		szInputPath = "";
	}
	else
	{
		szInputPath = szZipPath;
	}
	((StickerEffect*)handle)->loadEffectFromZip(szInputPath, szInputFile, callback, lpParam);
}

DX11IMAGEFILTER_EXPORTS_API bool ccStickerSetEffectZip_aysn(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam)
{
	assert(handle != 0);

	string szInputFile;
	if (szXML == NULL)
	{
		szInputFile = "";
	}
	else
	{
		szInputFile = szXML;
	}

	string szInputPath;
	if (szZipPath == NULL)
	{
		szInputPath = "";
	}
	else
	{
		szInputPath = szZipPath;
	}
	return ((StickerEffect*)handle)->loadEffectFromZip_aysn(szInputPath, szInputFile, callback, lpParam);
}


DX11IMAGEFILTER_EXPORTS_API void ccStickerSetEffectCacheSize(cc_handle_t handle, int nSize)
{
	assert(handle != 0);
	((StickerEffect*)handle)->setMaxCacheSize(nSize);
}

DX11IMAGEFILTER_EXPORTS_API void ccStickerDestory(cc_handle_t handle)
{
	assert(handle != 0);
	delete ((StickerEffect*)handle);
}

DX11IMAGEFILTER_EXPORTS_API void ccStickerResetAnimation(cc_handle_t handle)
{
	((StickerEffect*)handle)->resetAnimation();
}