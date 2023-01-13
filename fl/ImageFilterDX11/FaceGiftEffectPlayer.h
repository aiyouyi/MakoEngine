#pragma once

#include "common.h"
#include "DX11ImageFilterDef.h"
#include "FaceDetectorInterface.h"
#include "BodyDetectInterface.h"
#include <d3d11.h>

typedef void (*GiftEffectCallback)(const char *giftId);

#define MASK_TYPE 1
struct SMaskInfo
{
	unsigned char type;
	unsigned char *pMask;
	int maskW;
	int maskH;
};

#define AVANTAR_TYPE 2
struct SAvantarInfo
{
	unsigned char type;
	float arrAlpha[51];
};

DX11IMAGEFILTER_EXPORTS_API void ccSetGiftResourcePath(const char *szPath);
DX11IMAGEFILTER_EXPORTS_API void ccStartGiftEffect(const char *szGift, const char *sGiftId, GiftEffectCallback callback=NULL, bool bCache = true);
DX11IMAGEFILTER_EXPORTS_API ID3D11ShaderResourceView *ccPlayGiftEffect(ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes);
DX11IMAGEFILTER_EXPORTS_API bool ccPlayGiftEffectExt(ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes);
DX11IMAGEFILTER_EXPORTS_API bool ccPlayGiftEffectExt2(ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo);
DX11IMAGEFILTER_EXPORTS_API void ccSetBGRAGiftEffect(unsigned char *pBGRA);
DX11IMAGEFILTER_EXPORTS_API void ccDestoryGiftEffect();
DX11IMAGEFILTER_EXPORTS_API void ccSetGiftEffectCacheSize(int nSize);
DX11IMAGEFILTER_EXPORTS_API void ccDestoryGiftInst();
DX11IMAGEFILTER_EXPORTS_API void ccEffectSetGiftAnchor(unsigned char* data, int width, int height, AnchorType type);
DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBodyPointOld(ccBodyRes* bodyRes);
DX11IMAGEFILTER_EXPORTS_API void ccEffectSetSplitScreenOld(int SplitScreen, CCEffectType type);