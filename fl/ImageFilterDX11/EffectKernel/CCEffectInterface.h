#pragma once
#include "FaceDetectorInterface.h"
#include "HandDetectInterface.h"
#include "BodyDetectInterface.h"
#include "CEffectPart.h"
#include "Toolbox/RectDraw.h"
#include "common.h"
#include "BaseRenderParam.h"
#include <vector>
#include "Toolbox/zip/unzip.h"
#include "DX11ImageFilterDef.h"
#include "EffectDataDefine.h"

class DX11DoubleBuffer;

class DX11IMAGEFILTER_EXPORTS_CLASS CCEffectInterface
{
public:
	CCEffectInterface();
	~CCEffectInterface();

	bool loadEffectFromZipAsyn(const std::string&szZipFile, const std::string&xml, EffectCallback callback = 0, void *lpParam = 0);
	bool loadEffectFromZipSync(const std::string&szZipFile, const std::string&xml, EffectCallback callback = 0, void *lpParam = 0);

	bool AddEffectFromXML(const std::string&dir, const std::string&xml);

	bool renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes);
	bool renderEffectToTexture(ID3D11ShaderResourceView* pInputTexture, ID3D11Texture2D* pTargetTexture, int width, int height, const ccFDFaceRes* faceRes,bool needToCopyInputTexture,bool needToClearBuffer);

	ID3D11ShaderResourceView * renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, int width, int height, const ccFDFaceRes *faceRes);
	void SetMask(unsigned char* pMask, int nWidth, int nHeight, CCEffectType type = UNKNOW_EFFECT, AnchorType anchortype = ANCHOR_UNKNOW);
	void SetMakeUpZip(const std::string&szZipFile);

	void SetHairMask(unsigned char * pMask, int nWidth, int nHeight, CCEffectType type = UNKNOW_EFFECT);
	void SetBGRA(unsigned char *pRGBA);
	void SetRotate(float x, float y, float z, CCEffectType type = PBR_3D_MODEL);
	void SetColor(float r, float g, float b, float a, CCEffectType type = FACE_LIPSTRICK);
	void setAlpha(float Alpha, CCEffectType type);
	void SetSplitScreen(int SplitScreen, CCEffectType type);
	void SetHand(ccHandRes* handRes);
	void SetBodyPoint(ccBodyRes * bodyRes);

	void SetBlendShapeParam(float *pBlendShapeParam);

	void SetResourcePath(const char *path);
	void Realese();


	void* getCEffectParts(int *size) {
		if (nullptr == size)	return nullptr;
		*size = m_AllEffectPart.size();
		if (0 == *size) return nullptr;
		return m_AllEffectPart.data();
	}

	void zipAllResource(std::string &zipPath, std::string &tempPath, long duringtime = -1, const char* version = "1.2");
	void ZipAllResourceOnly(const std::string& ZipPath,const std::string& ContanFolder);
	void writeAllResource(std::string &tempPath, const char* version ="1.2");
	void WriteAllXMLConfig(const std::string& Path, const std::string& Name, const std::string& Version = "1.2");
	void UpdateTestXmlWithNewAttr(XMLNode& TestXMLNode);
	void addEffect(CEffectPart* part);
	void deleteEffect(CEffectPart* part);
	bool exchangePart(int src_index,int dst_index);
	bool MovePart(int src_index, int dst_index);

	std::vector<CEffectPart*> m_AllEffectPart;
	
private:

	void renderEffect(ID3D11ShaderResourceView *pInputTexture, int width, int height, const ccFDFaceRes *faceRes, bool needToCopyInputTexture, bool needToClearBuffer);
	void loadEffectFromZip_asyn_impl(const char *szPath, const char *szXMLFile);
	void loadEffectFromZip_sync_impl(const char *szPath, const char *szXMLFile);
	void touchThreadSource();
	void RealeseEffectPart();
	void RealeseCardId();
private:
	std::shared_ptr<DX11DoubleBuffer> m_DoubleBuffer;
	RectDraw *m_rectDraw;

	std::shared_ptr<CC3DTextureRHI> m_pTargetTextureA;
	std::shared_ptr<CC3DTextureRHI> m_pTargetTextureB;

	std::shared_ptr<CC3DTextureRHI> m_pTextureMask;
	std::shared_ptr<CC3DTextureRHI> m_pTextureHairMask;

	ccHandRes*m_pHandRes;
	ccBodyRes * m_pBodyRes;
	int m_SplitScreen = 1;

	DXGI_FORMAT m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CCEffect_status m_effectStatus = CES_IDLE;
	void *m_lpParam;
	EffectCallback m_pEffectCallback;

	BaseRenderParam *m_renderParam = nullptr;

	CCResourceAsyn m_resourceAsyn;
	HANDLE m_hLoadThread =0;

	std::string m_resourcePath;

	std::string m_LastZip = "";

	__int64 m_nStartTime;
	bool m_MSAA = false;

	std::map<AnchorType, long long> m_CardMask;
	std::shared_ptr<CC3DTextureRHI> m_InputSRV;
};

