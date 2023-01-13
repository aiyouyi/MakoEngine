#include "EffectCInterface.h"
#include "EffectKernel/CCEffectInterface.h"
#include "Toolbox/inc.h"
#include "HandDetectInterface.h"

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccEffectCreate()
{
	return (cc_handle_t)(new CCEffectInterface());
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectRelease(cc_handle_t handle) {
	assert(handle != 0);
	delete (CCEffectInterface*)handle;
}

DX11IMAGEFILTER_EXPORTS_API ID3D11ShaderResourceView * ccEffectProcess(cc_handle_t handle, ID3D11ShaderResourceView * pTexture, int width, int height, const ccFDFaceRes * faceRes)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->renderEffectToTexture(pTexture, width, height, faceRes);
}
DX11IMAGEFILTER_EXPORTS_API bool ccEffectProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height, const ccFDFaceRes *faceRes)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->renderEffectToTexture(pTexture, pDestTexture, width, height, faceRes);

}

DX11IMAGEFILTER_EXPORTS_API bool ccEffectProcessTexture2(cc_handle_t handle, ID3D11ShaderResourceView* pTexture, ID3D11Texture2D* pDestTexture, int width, int height, const ccFDFaceRes* faceRes)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->renderEffectToTexture(pTexture, pDestTexture, width, height, faceRes,false,true);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetMask(cc_handle_t handle, unsigned char *pMask, int width, int height, CCEffectType type, AnchorType anchortype)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetMask(pMask, width, height, type, anchortype);

}

void ccEffectSetMakeUpZip(cc_handle_t handle, const char * szZipPath)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetMakeUpZip(szZipPath);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetHairMask(cc_handle_t handle, unsigned char *pMask, int width, int height, CCEffectType type)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetHairMask(pMask, width, height, type);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetHand(cc_handle_t handle, ccHandRes_t* handRes)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetHand(handRes);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBodyPoint(cc_handle_t handle, ccBodyRes * bodyRes)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetBodyPoint(bodyRes);
}

void ccEffectSetBGRA(cc_handle_t handle, unsigned char * pBGRA)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetBGRA(pBGRA);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetColor(cc_handle_t handle, float r, float g, float b, float a, CCEffectType type)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetColor(r, g, b, a, type);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetRotate(cc_handle_t handle, float x, float y, float z, CCEffectType type )
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetRotate(x, y, z, type);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetAlpha(cc_handle_t handle, float alpha, CCEffectType type)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->setAlpha(alpha,type);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetSplitScreen(cc_handle_t handle, int SplitScreen, CCEffectType type)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetSplitScreen(SplitScreen, type);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetPath(cc_handle_t handle, const char* path)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetResourcePath(path);
}

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBlendShape(cc_handle_t handle, float *pBlendShapeCoeffs)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->SetBlendShapeParam(pBlendShapeCoeffs);
}

DX11IMAGEFILTER_EXPORTS_API bool ccEffectSetEffectZipAsyn(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam)
{
	assert(handle != 0);

	std::string szInputFile;
	if (szXML == NULL)
	{
		szInputFile = "";
	}
	else
	{
		szInputFile = szXML;
	}

	std::string szInputPath;
	if (szZipPath == NULL)
	{
		szInputPath = "";
	}
	else
	{
		szInputPath = szZipPath;
	}
	return ((CCEffectInterface*)handle)->loadEffectFromZipAsyn(szInputPath, szInputFile, callback, lpParam);
}

DX11IMAGEFILTER_EXPORTS_API bool ccEffectSetEffectZipSync(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam)
{
	assert(handle != 0);

	std::string szInputFile;
	if (szXML == NULL)
	{
		szInputFile = "";
	}
	else
	{
		szInputFile = szXML;
	}

	std::string szInputPath;
	if (szZipPath == NULL)
	{
		szInputPath = "";
	}
	else
	{
		szInputPath = szZipPath;
	}
	return ((CCEffectInterface*)handle)->loadEffectFromZipSync(szInputPath, szInputFile, callback, lpParam);
}

DX11IMAGEFILTER_EXPORTS_API bool ccEffectAddEffectFromXML(cc_handle_t handle, const char* szPath, const char* szXML)
{
	assert(handle != 0);

	std::string szInputFile;
	if (szXML == NULL)
	{
		szInputFile = "";
	}
	else
	{
		szInputFile = szXML;
	}

	std::string szInputPath;
	if (szPath == NULL)
	{
		szInputPath = "";
	}
	else
	{
		szInputPath = szPath;
	}
	return ((CCEffectInterface*)handle)->AddEffectFromXML(szPath, szXML);
}

void* ccGetCEffectParts(cc_handle_t handle, int *size)
{
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->getCEffectParts(size);
}

void ccZipAllEffect(cc_handle_t handle, const char *zipPath, const char *tempPath, long duringtime, const char* version) {
	assert(handle != 0);

	std::string szZipPath;
	std::string szTempPath;
	if (zipPath == NULL)
	{
		szZipPath = "";
		szTempPath = "";
	}
	else
	{
		szZipPath = zipPath;
		szTempPath = tempPath;
	}
	return ((CCEffectInterface*)handle)->zipAllResource(szZipPath, szTempPath, duringtime, version);
}

void ccWriteAllEffect(cc_handle_t handle, const char *tempPath, const char* version) {
	assert(handle != 0);
	
	std::string szTempPath;
	if (tempPath == NULL)
	{
		szTempPath = "";
	}
	else
	{
		szTempPath = tempPath;
	}
	return ((CCEffectInterface*)handle)->writeAllResource(szTempPath, version);
}

void ccAddEffect(cc_handle_t handle, void* part) {
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->addEffect((CEffectPart*)part);
}

void ccDeleteEffect(cc_handle_t handle, void* part) {
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->deleteEffect((CEffectPart*)part);
}

bool ccExchangePart(cc_handle_t handle, int src_index, int dst_index) {
	assert(handle != 0);
	return ((CCEffectInterface*)handle)->exchangePart(src_index, dst_index);
}
