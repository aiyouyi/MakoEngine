#pragma once
#include <vector>
#include <string>
#include "BaseDefine/Define.h"
#include "common.h"
#include "FaceDetectorInterface.h"
#include "EffectDataDefine.h"
#include "EffectKernel/CEffectPart.h"

class GLDoubleBuffer;
class BaseRenderParam;


class DX11IMAGEFILTER_EXPORTS_CLASS CCEffectInterfaceGL
{
public:
	CCEffectInterfaceGL();
	~CCEffectInterfaceGL();

	bool loadEffectFromZipAsyn(const std::string &szZipFile, const std::string &xml, EffectCallback callback = 0, void *lpParam = 0);
	bool loadEffectFromZipSync(const std::string & szZipFile, const std::string & xml, EffectCallback callback = 0, void *lpParam = 0);
    bool loadBaseFromZipSync(const std::string &szZipFile, const std::string &xml);
	bool renderEffectToTexture(GLuint pInputTexture, GLuint pTargetTexture, int width, int height, const ccFDFaceRes *faceRes ,FaceOriImageRect *pFaceRect = NULL);
	void SetBGRA(unsigned char *pRGBA);
	void SetMatrial(unsigned char *pMaterial, int nWidth, int nHeight, CCEffectType type = LUT_FILTER_EFFECT);
	void SetMatrial(const char *fileName, CCEffectType type = LUT_FILTER_EFFECT);
	void SetMakeUpZip(const std::string &szZipFile);//替换美妆素材配置包

	void SetExpression(std::vector<float>& coeffs);

	//void SetAudio(float sampleRate, CCEffectType type = WALLPAPER_PBRMODEL);
	//void AnalyseAudio(float* buffer, int length, CCEffectType type = WALLPAPER_PBRMODEL);
	//void EndAudio(CCEffectType type = WALLPAPER_PBRMODEL);
	//void ZoomInOut(float scale, CCEffectType type = WALLPAPER_PBRMODEL);

	void SetColor(float r, float g, float b, float a, CCEffectType type = FACE_LIPSTRICK);
	void SetResourcePath(char *path);
	//void SetRotate(float x, float y, float z, CCEffectType type = PBR_3D_MODEL);
	void SetIOSDevice(bool isIpad, CCEffectType type);
	void Realese();

    void setAlpha(float Alpha,CCEffectType type);
    float GetAlpha(CCEffectType type);
	void setBlend(BlendType blendType, CCEffectType type);
	void setBlendType(int index, CCEffectType type);
	void RealeseEffectPart();
    
    void facePointIndexTransfer(const ccFDFaceRes* faceRes, std::vector<float>& mDstPoint74);

private:
	void renderEffect(GLuint pInputTexture, int width, int height, const ccFDFaceRes *faceRes);
	void loadEffectFromZip_asyn_impl(const char *szPath, const char *szXMLFile);
	void loadEffectFromZip_sync_impl(const char *szPath, const char *szXMLFile);
	void touchThreadSource();
	void *ReadConfigThread(void *pM);
    void RealeseBasePart();
	
private:
	CCEffect_status m_effectStatus;
	void *m_lpParam;
	EffectCallback m_pEffectCallback;
	std::vector<CEffectPart*> m_AllEffectPart;
    std::vector<CEffectPart*> m_BasePart;
	BaseRenderParam *m_renderParam;
    bool m_useDepth = false;
    bool m_lastUseDepth =false;
	//�̼߳���
	CCResourceAsyn m_resourceAsyn;
    //HANDLE m_hLoadThread;

	std::string m_resourcePath;
	bool m_ReleaseStatus = false;
	std::string m_LastZip;

	long long  m_nStartTime;
	std::vector<float> m_Coffes;
	std::shared_ptr<GLDoubleBuffer> m_DoubleBuffer;
};

