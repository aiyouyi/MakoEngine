#pragma once
#include "FaceDetectorInterface.h"
#include "CCHandGestureInterface.h"
#include "CEffectPart.h"
#include "Toolbox/RectDraw.h"
#include "common.h"
#include "BaseRenderParam.h"
#include <vector>
#include "Toolbox/zip/unzip.h"
#include "DX11ImageFilterDef.h"


enum CCEffect_status
{
	CES_IDLE,
	CES_LOADING,
	CES_READY,
	CES_RUN
};


struct CCResourceAsyn
{
	void reset()
	{
		//input
	//	Release();
		szPath = "";
		szXMLFile = "";
		status = CES_IDLE;
	}
	void Release()
	{
		for (int i = 0; i < m_AllEffectPart.size(); ++i)
		{
			SAFE_DELETE(m_AllEffectPart[i]);
		}
		m_AllEffectPart.clear();
	}
	//input
	string szPath;
	string szXMLFile;
	string m_resourcePath;

	CCEffect_status status;
	std::vector<CEffectPart*>m_AllEffectPart;
	long long m_nEffectDuring;

};

class DX11IMAGEFILTER_EXPORTS_CLASS CCEffectInterface
{
public:
	CCEffectInterface();
	~CCEffectInterface();

	bool loadEffectFromZipAsyn(const string &szZipFile, const string &xml, EffectCallback callback = 0, void *lpParam = 0);
	bool loadEffectFromZipSync(const string &szZipFile, const string &xml, EffectCallback callback = 0, void *lpParam = 0);

	bool AddEffectFromXML(const string &dir, const string &xml);

	bool renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes);
	bool renderEffectToTexture(ID3D11ShaderResourceView* pInputTexture, ID3D11Texture2D* pTargetTexture, int width, int height, const ccFDFaceRes* faceRes,bool needToCopyInputTexture,bool needToClearBuffer);

	ID3D11ShaderResourceView * renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, int width, int height, const ccFDFaceRes *faceRes);
	void SetMask(unsigned char *pMask, int nWidth,int nHeight,CCEffectType type = UNKNOW_EFFECT);

	void SetMakeUpZip(const string &szZipFile);

	void SetHairMask(unsigned char * pMask, int nWidth, int nHeight, CCEffectType type = UNKNOW_EFFECT);
	void SetBGRA(unsigned char *pRGBA);
	void SetRotate(float x, float y, float z, CCEffectType type = PBR_3D_MODEL);
	void SetColor(float r, float g, float b, float a, CCEffectType type = FACE_LIPSTRICK);
	void setAlpha(float Alpha, CCEffectType type);
	void SetHand(ccHGHandRes * handRes);
	void SetBlendShapeParam(float *pBlendShapeParam);

	void SetResourcePath(char *path);
	void Realese();


	void* getCEffectParts(int *size) {
		if (nullptr == size)	return nullptr;
		*size = m_AllEffectPart.size();
		if (0 == *size) return nullptr;
		return m_AllEffectPart.data();
	}

	void zipAllResource(std::string &zipPath, std::string &tempPath, long duringtime = -1, const char* version = "1.2");
	void writeAllResource(std::string &tempPath, const char* version ="1.2");
	void addEffect(CEffectPart* part);
	void deleteEffect(CEffectPart* part);
	bool exchangePart(int src_index,int dst_index);
	bool MovePart(int src_index, int dst_index);

	std::vector<CEffectPart*> m_AllEffectPart;
	bool m_MSAA = false;
private:

	void renderEffect(ID3D11ShaderResourceView *pInputTexture, int width, int height, const ccFDFaceRes *faceRes, bool needToCopyInputTexture, bool needToClearBuffer);
	void loadEffectFromZip_asyn_impl(const char *szPath, const char *szXMLFile);
	void loadEffectFromZip_sync_impl(const char *szPath, const char *szXMLFile);
	void touchThreadSource();
	void RealeseEffectPart();
private:
	DX11DoubleBuffer *m_DoubleBuffer;
	RectDraw *m_rectDraw;

	DX11Texture *m_pTargetTextureA;
	DX11Texture *m_pTargetTextureB;

	DX11Texture *m_pTextureMask;
	DX11Texture *m_pTextureHairMask;
	ccHGHandRes *m_pHandRes;

	DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CCEffect_status m_effectStatus;
	void *m_lpParam;
	EffectCallback m_pEffectCallback;

	BaseRenderParam *m_renderParam;
	//�̼߳���
	CCResourceAsyn m_resourceAsyn;
	HANDLE m_hLoadThread =0;

	std::string m_resourcePath;

	std::string m_LastZip = "";

	__int64 m_nStartTime;



};

