#pragma once
#include "Toolbox/EffectModel.hpp"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include "Toolbox/RectDraw.h"
#include <xnamath.h>
#include "FaceDetectorInterface.h"
#include "common.h"
#include <list>
#include "FacialAnimation.h"
#include "Toolbox/xmlParser.h"
#include "Toolbox/zip/unzip.h"

struct StickerConstantBuffer
{
	Mat4 mWVP; //混合矩阵
	XMVECTOR mColor;
};

struct HeaderForCullConstantBuffer
{
	Mat4 mWVP; //混合矩阵
};

/*
enum ResourceLoadStatus
{
	RLS_START,
	RLS_FINISHED,
	IDLE   //酱油状态
};
*/

enum StickerEffect_status
{
	SES_IDLE,
	SES_LOADING,
	SES_READY,
	SES_RUN
};

struct Cocos3DScene
{
public:
	bool loadFromXML(const XMLNode &nodeModels, HZIP hZip, const char *szFile);

	bool needRender()
	{
		return m_vRootObjects.size() > 0;
	}

	void destory();

	void reset();
public:
	std::vector<FSObject *> m_vRootObjects;
	FacialObject *m_pFacialObj;

	//灯光
	std::vector<BaseLight> m_vLights;
};

struct SEffectInfo
{
public:
	void destory();
	bool hasObject();

	void reset();
public:
	std::vector<EffectModel> m_vEffectModel;
	std::vector<Effect2DRect> m_v2DEffectModel;
	Drawable *m_v2DBGAnimation;
	Cocos3DScene m_cocosScene;
};

struct SResourceAsyn
{
	void reset()
	{
		//input
		szPath = "";
		szXMLFile = "";
		szDesc = "";

		//output
		m_effectInfo.reset();

		effectLoopMode = ELM_REPEAT;

		m_nEffectDuring = -1;
		m_nAniLoopTime = -1;


		status = SES_IDLE;
	}

	//input
	std::string szPath;
	std::string szDesc;
	std::string szXMLFile;

	//output
	SEffectInfo m_effectInfo;

	en_EffectLoop_Mode effectLoopMode;
	long m_nEffectDuring;
	long m_nAniLoopTime;

	StickerEffect_status status;
};

struct SEffectCache
{
	SEffectInfo m_effectInfo;

	std::string szPath;
	std::string szXMLFile;
	long nEffectDuring;
	long nAniLoopTime;
	en_EffectLoop_Mode effectLoopMode;
};

typedef bool(*LayerRenderFunc)(int type);

class StickerEffect
{
public:
	StickerEffect();
	virtual ~StickerEffect();

	void setMaxCacheSize(int nSize);

	bool loadEffect(const string &szPath, const string &file, EffectCallback callback=0, void *lpParam=0);
	bool loadEffectFromZip(const string &szZipFile, const string &xml, EffectCallback callback = 0, void *lpParam = 0);
	bool loadEffectFromZip_aysn(const string &szZipFile, const string &xml, EffectCallback callback = 0, void *lpParam = 0);
	void destroyEffect();
	void destroyCacheEffect();

	void resetAnimation();

	void destory();

	void renderEffect(int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo = NULL, int *arrRenderView = NULL);

	ID3D11ShaderResourceView *renderEffectToTexture(ID3D11ShaderResourceView *pTexture, int width, int height, const ccFDFaceRes *faceRes);

	bool renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes, void *pExtInfo=NULL);
private:
	bool prepare();
	bool loadEffect_impl(const char *szPath, const char *szXMLFile);
	void loadEffectFromZip_impl(const char *szPath, const char *szXMLFile);
	void loadEffectFromZip_asyn_impl(const char *szPath, const char *szXMLFile);
	void touchThreadSource();

	void RecycleEffect();
	bool loadFromCache(const std::string &szZipFile, const std::string &xml);

	void render2DEffect(int width, int height, long during, LayerRenderFunc fRender);
	void render3DSEffect(int width, int height, long during, LayerRenderFunc fRender, const ccFDFaceRes *faceRes, void *pExtInfo=NULL);

	ID3D11ShaderResourceView* GetSRV(std::shared_ptr<CC3DTextureRHI> TexRHI);

private:
	DX11FBO *m_pFBO;
	RectDraw *m_rectDraw;

	DX11Texture *m_pTargetTexture;

	bool m_bPrepare;

	StickerEffect_status m_effectStatus;
	long m_nEffectDuring;
	long m_nAniLoopTime;
	void *m_lpParam;
	EffectCallback m_pEffectCallback;
	en_EffectLoop_Mode m_effectLoopMode;

	SEffectInfo m_effectInfo;

	std::string m_szCurrentDir;
	std::string m_szCurrentXML;


	std::list<SEffectCache *> m_listEffectCache;
	int m_nMaxCacheSize;

	//人头像模型
	static std::vector<HeaderModel> m_vHeaderModel;

	//线程加载
	SResourceAsyn m_resourceAsyn;
	HANDLE m_hLoadThread = nullptr;

	//渲染模型信息
	DX11Shader *m_pShader;
	ID3D11Buffer*       m_pConstantBuffer;  //转换矩阵

	//渲染人头像模型
	DX11Shader *m_pShaderForHeaderCull;
	ID3D11Buffer *m_pConstantBufferForHeaderCull;  //转换矩阵

	//采样状态
	ID3D11SamplerState* m_pSamplerLinear;

	//混合以及buffer操作
	ID3D11BlendState *m_pBSEnable;
	ID3D11BlendState *m_pBSWriteDisable;
	ID3D11BlendState *m_pBSDisable;
	ID3D11BlendState *m_pBSDisableWriteDisable;

	//深度操作
	ID3D11DepthStencilState *m_pDepthStateEnable;
	ID3D11DepthStencilState *m_pDepthStateDisable;
	ID3D11DepthStencilState *m_pDepthStateEnableWriteDisable;

	__int64 m_nStartTime;
	__int64 m_nLoopStartTime;
	__int64 m_nLastTime;

	std::shared_ptr<class CC3DTextureRHI> m_InputSRV;
};

