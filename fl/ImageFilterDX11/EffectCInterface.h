#pragma once

#include "common.h"
#include "DX11ImageFilterDef.h"
#include "FaceDetectorInterface.h"

#include "BodyDetectInterface.h"
#include <d3d11.h>
#include <vector>

struct ccHandRes_t;

DX11IMAGEFILTER_EXPORTS_API cc_handle_t ccEffectCreate();
DX11IMAGEFILTER_EXPORTS_API void ccEffectRelease(cc_handle_t);

DX11IMAGEFILTER_EXPORTS_API ID3D11ShaderResourceView *ccEffectProcess(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes);
DX11IMAGEFILTER_EXPORTS_API bool ccEffectProcessTexture(cc_handle_t handle, ID3D11ShaderResourceView *pTexture, ID3D11Texture2D *pDestTexture, int width, int height, const ccFDFaceRes *faceRes );
//����������ͼ������ֱ��������
DX11IMAGEFILTER_EXPORTS_API bool ccEffectProcessTexture2(cc_handle_t handle, ID3D11ShaderResourceView* pTexture, ID3D11Texture2D* pDestTexture, int width, int height, const ccFDFaceRes* faceRes);

DX11IMAGEFILTER_EXPORTS_API bool ccEffectSetEffectZipAsyn(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam);
DX11IMAGEFILTER_EXPORTS_API bool ccEffectSetEffectZipSync(cc_handle_t handle, const char *szZipPath, const char *szXML, EffectCallback callback, void *lpParam);

DX11IMAGEFILTER_EXPORTS_API bool ccEffectAddEffectFromXML(cc_handle_t handle, const char* szPath, const char* szXML);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetMask(cc_handle_t handle, unsigned char *pMask,  int width, int height, CCEffectType type = UNKNOW_EFFECT, AnchorType anchortype = ANCHOR_UNKNOW);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetMakeUpZip(cc_handle_t handle, const char *szZipPath);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetHairMask(cc_handle_t handle, unsigned char *pMask, int width, int height, CCEffectType type = UNKNOW_EFFECT);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetHand(cc_handle_t handle, ccHandRes_t* handRes);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBodyPoint(cc_handle_t handle, ccBodyRes * bodyRes);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBGRA(cc_handle_t handle, unsigned char *pBGRA);


DX11IMAGEFILTER_EXPORTS_API void ccEffectSetColor(cc_handle_t handle, float r, float g, float b, float a, CCEffectType type = FACE_LIPSTRICK);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetRotate(cc_handle_t handle, float x, float y, float z, CCEffectType type = PBR_3D_MODEL);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetPath(cc_handle_t handle, const char *path);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetBlendShape(cc_handle_t handle, float *pBlendShapeCoeffs);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetAlpha(cc_handle_t handle,float alpha, CCEffectType type);

DX11IMAGEFILTER_EXPORTS_API void ccEffectSetSplitScreen(cc_handle_t handle, int SplitScreen, CCEffectType type);


DX11IMAGEFILTER_EXPORTS_API void* ccGetCEffectParts(cc_handle_t handle, int *size);

DX11IMAGEFILTER_EXPORTS_API void ccZipAllEffect(cc_handle_t handle, const char *zipPath, const char *tempPath, long duringtime = -1, const char* version = "1.2");

DX11IMAGEFILTER_EXPORTS_API void ccWriteAllEffect(cc_handle_t handle, const char *tempPath, const char* version = "1.2");

DX11IMAGEFILTER_EXPORTS_API void ccAddEffect(cc_handle_t handle, void* part);
DX11IMAGEFILTER_EXPORTS_API void ccDeleteEffect(cc_handle_t handle, void* part);
DX11IMAGEFILTER_EXPORTS_API bool ccExchangePart(cc_handle_t handle, int src_index, int dst_index);

