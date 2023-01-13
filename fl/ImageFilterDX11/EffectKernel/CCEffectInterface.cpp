//#define VLD_FORCE_ENABLE
//#include"vld.h"
#include "CCEffectInterface.h"
#include "Toolbox/zip/unzip.h"
#include "Toolbox/zip/zip.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "BaseRenderParam.h"
#include "EffectKernel/FaceEffect/CFaceEffect2D.h"
#include "EffectKernel/FaceEffect/CFaceEffect2DWithBG.h"
#include "EffectKernel/FaceMakeUp/CFaceMakeUp.h"
#include "EffectKernel/FaceMakeUp/CFaceBlendMakeUp.h"
#include "EffectKernel/FaceLift/CFaceSPMLS.h"
#include "EffectKernel/FaceLift/CFaceMeshChange.h"
#include "EffectKernel/FaceLift/CFaceMeshChange2.h"
#include "EffectKernel/Filter/CDeNoiseFilterEffect.h"
#include "EffectKernel/FaceEffect/CFaceEffect3DModel.h"
#include "EffectKernel/Sticker/C2DPicTextureEffect.h"
#include "EffectKernel/Sticker/C2DPicTextureBlendEffect.h"
#include "EffectKernel/Sticker/CBodyBGEffect.h"
#include"EffectKernel/Sticker/CBGDraw.h"
#include"EffectKernel/Sticker/CFilterWithMask.h"
#include"EffectKernel/Sticker/CCopyTex.h"
#include"EffectKernel/Sticker/CFaceEffect2DSticker.h"
#include "EffectKernel/Sticker/CPurpleFire2DEffect.h"
#include "EffectKernel/FaceEffect/CFaceSmoothIns.h"
#include"EffectKernel/Filter/CLUTFIlter.h"
#include "EffectKernel/Filter/CFoodieSharp.h"
#include "EffectKernel/Filter/CBackGroundFilter.h"
#include "EffectKernel/FaceEffect/CFaceShadowHighLight.h"
#include "EffectKernel/FaceLift/CFaceBeauty.h"
#include "EffectKernel/FaceLift/CBodyLift.h"
#include "EffectKernel/FaceLift/CBodyLiftYY.h"
#include "EffectKernel/FaceMakeUp/CFaceLipstick.h"
#include "EffectKernel/FaceBeauty/CFaceLevel.h"
#include "EffectKernel/FaceBeauty/CFaceEffect2DBlendBG.h"
#include "EffectKernel/FaceEffect/CFaceEffect3DRect.h"
#include "EffectKernel/HandEffect/CHandEffectHeart.h"
#include "EffectKernel/PBR/CFacePBRModel.h"
#include "EffectKernel/Sticker/CBodyTrack2DEffect.h"
#include "EffectKernel/Sticker/CBodyFGTrackEffect.h"
#include "EffectKernel/PBR/CFaceBodyApartPBRModel.h"
#include "EffectKernel/FaceEffect/CFaceRemovePouchFalin.h"
#include "EffectKernel/LensEffect/CSoulBody.h"
#include "EffectKernel/LensEffect/CGlitch.h"
#include "EffectKernel/LensEffect/CLightWave.h"
#include "EffectKernel/FaceEffect/CFaceEffectFlipSplit.h"
#include "EffectKernel/LensEffect/CRhythm.h"
#include "EffectKernel/FaceBeauty/CDyeHairEffect.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "EffectKernel/ResourceManager.h"
#include "Toolbox\fileSystem.h"
#include "ShaderProgramManager.h"
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "EffectKernel/FaceEffect/CFaceEffect3DNormal.h"
#include "EffectKernel/ParticleEffect/CParticleEffect.h"
#include "EffectKernel/ParticleEffect/CHandParticleEffect.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/DXUtils/DX11DynamicRHI.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/string_util.h"
#include "EffectKernel/FaceLift/CBodyLift2.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "CC3DEngine/Material/CC3DMaterial.h"



CCEffectInterface::CCEffectInterface()
{
	m_lpParam = NULL;
	m_pEffectCallback = NULL;
	m_rectDraw = NULL;

	m_pTargetTextureA = NULL;
	m_pTargetTextureB = NULL;

	m_DoubleBuffer = NULL;

	m_renderParam = NULL;

	m_pTextureMask = NULL;
	m_pTextureHairMask = NULL;
	m_resourcePath = "./";
	m_pHandRes = NULL;
	m_pBodyRes = NULL;
}


CCEffectInterface::~CCEffectInterface()
{
	Realese();
}


void CCEffectInterface::Realese()
{
	//m_resourceAsyn.Release(); //it was released by RealeseEffectPart();
	SAFE_DELETE(m_rectDraw);
	SAFE_DELETE(m_renderParam);

	m_DoubleBuffer.reset();
	m_pTargetTextureA.reset();
	m_pTargetTextureB.reset();
	m_pTextureMask.reset();
	m_pTextureHairMask.reset();

	RealeseEffectPart(); 
	RealeseCardId();
	CC3DEnvironmentConfig::Release();
	ShaderProgramManager::GetInstance()->Release();
	ShaderProgramManager::ReleaseInstance();

	ResourceManager::Instance().Release();

}

void CCEffectInterface::RealeseEffectPart()
{

	for (int i = 0; i < m_AllEffectPart.size(); ++i)
	{
		SAFE_DELETE(m_AllEffectPart[i]);
	}
	m_AllEffectPart.clear();
}

void  CCEffectInterface::RealeseCardId()
{
	std::map<AnchorType, long long>::iterator iter;
	for (iter = m_CardMask.begin(); iter != m_CardMask.end(); ++iter)
	{
		if (iter->first != AnchorType::ANCHOR_UNKNOW)
		{
			ResourceManager::Instance().freeMaterial(iter->second);
		}
	}
}


CEffectPart * GetPart(const char *szType)
{
	CEffectPart *pPart = NULL;
	if (szType != NULL && !strcmp(szType, "FaceEffect2D"))
	{
		pPart = new CFaceEffect2D();
		pPart->m_MSAA = true;
	}
	else if (szType != NULL && !strcmp(szType, "FaceEffect2DSticker"))
	{
		pPart = new CFaceEffect2DSticker();
	}
	else if (szType != NULL && !strcmp(szType, "FaceEffect2DWithBG"))
	{
		pPart = new CFaceEffect2DBlendBG();
	}
	else if (szType != NULL && !strcmp(szType, "FlipSplit"))
	{
		pPart = new CFaceEffectFlipSplit();
	}
	else if (szType != NULL && !strcmp(szType, "FaceBlush"))
	{
		pPart = new CFaceEffect2DBlendBG();
		CFaceEffect2DBlendBG *p = (CFaceEffect2DBlendBG*)pPart;
		p->m_EffectPart = FACE_BLUSH;
		p->m_alpha = 0.0;
	}
	else if (szType != NULL && !strcmp(szType, "FaceBlendBlush"))
	{
		//�����������Ҫ����۲������Ч��
		pPart = new CFaceEffect2DBlendBG();
		CFaceEffect2DBlendBG *p = (CFaceEffect2DBlendBG*)pPart;
		p->m_EffectPart = FACE_BLEND_BLUSH;
		p->m_alpha = 0.0;

	}
	else if (szType != NULL && !strcmp(szType, "MakeUpBlend"))
	{
		pPart = new CFaceBlendMakeUp();
		CFaceBlendMakeUp *p = (CFaceBlendMakeUp*)pPart;
		p->m_EffectPart = FACE_EYESTRICK;
	}
	else if (szType != NULL && !strcmp(szType, "FaceMakeUp"))
	{
		pPart = new CFaceMakeUp();
		CFaceMakeUp *p = (CFaceMakeUp*)pPart;
		p->m_EffectPart = FACE_EYESTRICK;
	}
	else if (szType != NULL && !strcmp(szType, "MakeUp"))
	{
		pPart = new CFaceMakeUp();
	}
	else if (szType != NULL && !strcmp(szType, "FaceMLS"))
	{
		pPart = new CFaceSPMLS();
	}
	else if (szType != NULL && !strcmp(szType, "FaceMeshChange"))
	{
		pPart = new CFaceMeshChange();
	}
	else if (szType != NULL && !strcmp(szType, "FaceMeshChange2"))
	{
		pPart = new CFaceMeshChange2();
	}
	else if (szType != NULL && !strcmp(szType, "Denoise"))
	{
		pPart = new CDeNoiseFilterEffect();
	}
	else if (szType != NULL && !strcmp(szType, "2DTexture"))
	{
		pPart = new C2DPicTextureEffect();
	}
	else if (szType != NULL && !strcmp(szType, "2DTextureNew"))
	{
		pPart = new C2DPicTextureEffect();
	}
	else if (szType != NULL && !strcmp(szType, "2DTextureBlend"))
	{
		pPart = new C2DPicTextureBlendEffect();
	}
	else if (szType != NULL && !strcmp(szType, "FaceEffect3DModel"))
	{
		pPart = new CFaceEffect3DModel();
		pPart->m_MSAA = true;

	}
	else if (szType != NULL && !strcmp(szType, "FaceEffect3DRect"))
	{
		pPart = new CFaceEffect3DRect();
		pPart->m_MSAA = true;

	}
	else if (szType != NULL && !strcmp(szType, "BodyBGEffect"))
	{
		pPart = new CBodyBGEffect();

	}
	else if (szType != NULL && !strcmp(szType, "BodyTrack2DEffect"))
	{
		pPart = new CBodyTrack2DEffect();

	}
	else if (szType != NULL && !strcmp(szType, "CBodyFGTrackEffect"))
	{
		pPart = new CBodyFGTrackEffect();

	}
	else if (szType != NULL && !strcmp(szType, "PurpleFire2DEffect"))
	{
		pPart = new CPurpleFire2DEffect();
	}
	else if (szType != NULL && !strcmp(szType, "BackGroundFilterEffect"))
	{
		pPart = new CBackGroundFilter();

	}
	else if (szType != NULL && !strcmp(szType, "FilterWithMask"))
	{
		pPart = new CFilterWithMask();

	}
	else if (szType != NULL && !strcmp(szType, "BGDraw"))
	{
		pPart = new CBGDraw();

	}
	else if (szType != NULL && !strcmp(szType, "CopyTex"))
	{
		pPart = new CCopyTex();

	}
	else if (szType != NULL && !strcmp(szType, "InsSmooth"))
	{
		pPart = new CFaceSmoothIns();
	}
	else if (szType != NULL && !strcmp(szType, "FaceLevel"))
	{
		pPart = new CFaceLevel();
	}
	else if (szType != NULL && !strcmp(szType, "LUTFilter"))
	{
		pPart = new CLUTFIlter();
	}
	else if (szType != NULL && !strcmp(szType, "TYPEFilter"))
	{
		pPart = new CLUTFIlter();
		pPart->m_EffectPart = TYPE_FILTER_EFFECT;
		pPart->m_alpha = 0.0;
	}
	else if (szType != NULL && !strcmp(szType, "FoodieSharp"))
	{
		pPart = new CFoodieSharp();
	}
	else if (szType != NULL && !strcmp(szType, "FaceHighlight"))
	{
		pPart = new CFaceShadowHighLight();
	}
	else if (szType != NULL && !strcmp(szType, "FaceShapeBeauty"))
	{
		pPart = new CFaceBeauty();
	}
	else if (szType != NULL && !strcmp(szType, "BodyLift"))
	{
		pPart = new CBodyLift();
	}
	else if (szType != NULL && !strcmp(szType, "BodyLiftYY"))
	{
	    pPart = new CBodyLiftYY();
	}
	else if (szType != NULL && !strcmp(szType, "BodyLift2"))
	{
	    pPart = new CBodyLift2();
	}
	else if (szType != NULL && !strcmp(szType, "FaceLipstick"))
	{
		pPart = new CFaceLipstick();
		CFaceLipstick *p = (CFaceLipstick*)pPart;
		p->m_alpha = 0.0;
	}
	else if (szType != NULL && !strcmp(szType, "FacePBRModel"))
	{
	    pPart = new CFacePBRModel();
		pPart->m_MSAA = true;
	}
	else if (szType != nullptr && !strcmp(szType, "ParticleEffect"))
	{
		pPart = new CParticleEffect();
		//pPart->m_MSAA = true;
	}
	else if (szType != nullptr && !strcmp(szType, "HandParticleEffect"))
	{
		pPart = new CHandParticleEffect();
	}
	else if (szType != nullptr && !strcmp(szType, "FaceBodyPBRModel"))
	{
		pPart = new CFaceBodyApartPBRModel();
		pPart->m_MSAA = true;
	}
	else if (szType != NULL && !strcmp(szType, "RemovePouchFalin"))
	{
		pPart = new CFaceRemovePouchFalin();
	}
	else if (szType != NULL && !strcmp(szType, "Soulbody"))
	{
	    pPart = new CSoulBody();
	}
	else if (szType != NULL && !strcmp(szType, "Rhythm"))
	{
		pPart = new CRhythm();
	}
	else if (szType != NULL && !strcmp(szType, "Glitch"))
	{
		pPart = new CGlitch();
	}
	else if (szType != NULL && !strcmp(szType, "LightWave"))
	{
	    pPart = new CLightWave();
	}
	else if (szType != NULL && !strcmp(szType, "DyeHair"))
	{
	    pPart = new CDyeHairEffect();
	}
	else if (szType != NULL && !strcmp(szType, "HandHeartGesture"))
	{
		pPart = new CHandEffectHeart();
	}
	else if (szType != NULL && !strcmp(szType, "FaceNormal"))
	{
		pPart = new CFaceEffect3DNormal();
		pPart->m_MSAA = true;
	}
	else
	{
		pPart = new CEffectPart();
	}
	return pPart;
}

DWORD WINAPI ReadConfigThread(LPVOID pM)
{
	CCResourceAsyn *pResourceAsyn = (CCResourceAsyn *)pM;
	const char *szPath = pResourceAsyn->szPath.c_str();
	const char *szXMLFile = pResourceAsyn->szXMLFile.c_str();

	std::wstring wPath = core::u8_ucs2(pResourceAsyn->szPath);

	std::ifstream inStream(wPath, ios_base::in | ios_base::binary);
	std::stringstream buffer;
	buffer << inStream.rdbuf();
	std::string contents(buffer.str());

	HZIP hZip = nullptr;
	if (contents.empty())
	{
		hZip = OpenZip(szPath, nullptr);
	}
	else
	{
		hZip = OpenZip((char*)contents.data(), contents.length(), nullptr);
	}

	if (hZip != NULL)
	{
		ZIPENTRY ze;
		int index;

		long effectDuring = -1;
		if ( ZR_OK == FindZipItem(hZip, "giftDesc.xml", true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res == ZR_OK)
			{
				XMLResults xResults;
				XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "giftDesc", &xResults);
				const char *effectXML = nodeModels.getAttribute("effectXML");
				const char *szDuring = nodeModels.getAttribute("during");
				if (effectXML != NULL)
				{
					pResourceAsyn->szXMLFile = effectXML;
					szXMLFile = pResourceAsyn->szXMLFile.c_str();
				}
				if (szDuring != NULL)
				{
					effectDuring = atoi(szDuring);
				}
			}
			
			delete[]pDataBuffer;
		}
		pResourceAsyn->m_nEffectDuring = effectDuring;
		if (ZR_OK == FindZipItem(hZip, pResourceAsyn->szXMLFile.c_str(), true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]pDataBuffer;
				CloseZip(hZip);
				pResourceAsyn->status = CES_READY;
				return 0;
			}

			XMLResults xResults;
			XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
			delete[]pDataBuffer;
			ResourceManager::Instance().CurrentTestXMLInZip = nodeModels;
			//Get version and Version control
			const char *szVersion = nodeModels.getAttribute("version");
			if (szVersion != NULL)
			{
				double m_EffectVersion = atof(szVersion);
				if (m_EffectVersion > LIBVERSION_DOUBLE(LIBVERSION))
				{
					CloseZip(hZip);
					pResourceAsyn->status = CES_READY;
					return 0;
				}
			}

			int i = -1;
			XMLNode nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
			while (!nodeEffect.isEmpty())
			{
				const char *szType = nodeEffect.getAttribute("type");
				auto pPart = GetPart(szType);
				pPart->SetResourcePath(pResourceAsyn->m_resourcePath);
				pPart->ReadConfig(nodeEffect, hZip, (char*)pResourceAsyn->szPath.c_str());
				pPart->Prepare();
				pResourceAsyn->m_AllEffectPart.push_back(pPart);


				nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
			}
		}
	}
	CloseZip(hZip);

	pResourceAsyn->status = CES_READY;
	return 1;

}


void CCEffectInterface::renderEffect(ID3D11ShaderResourceView * pInputTexture, int width, int height, const ccFDFaceRes * faceRes, bool needToCopyInputTexture, bool needToClearBuffer)
{
	if (m_nStartTime < 0)
	{
		m_nStartTime = ccCurrentTime();
	}

	__int64 currentTime = ccCurrentTime();
	long runTime = long(currentTime - m_nStartTime);

	if (m_resourceAsyn.m_nEffectDuring > 0 && runTime >= m_resourceAsyn.m_nEffectDuring)
	{
		m_effectStatus = CES_IDLE;
		if (m_pEffectCallback != NULL)
		{
			(*m_pEffectCallback)(EE_FINISHED, m_lpParam);
		}
		return;
	}

	if (m_DoubleBuffer == NULL || m_DoubleBuffer->GetWidth() != width || m_DoubleBuffer->GetHeight() != height || m_pTargetTextureA == NULL || m_MSAA !=m_DoubleBuffer->IsMsaa())
	{
		m_DoubleBuffer.reset();

		uint32_t format = CC3DTextureRHI::SFT_A8R8G8B8;

		m_pTargetTextureA = GetDynamicRHI()->CreateTexture(format, CC3DTextureRHI::OT_RENDER_TARGET, width, height, nullptr, 0, false, m_MSAA);
		m_pTargetTextureB = GetDynamicRHI()->CreateTexture(format, CC3DTextureRHI::OT_RENDER_TARGET, width, height, nullptr, 0, false, m_MSAA);

		m_DoubleBuffer = std::make_shared<DX11DoubleBuffer>();
		m_DoubleBuffer->InitDoubleBuffer(m_pTargetTextureA, m_pTargetTextureB, width, height, m_MSAA);

		SAFE_DELETE(m_renderParam);
		m_renderParam = new BaseRenderParam();
		m_renderParam->SetDoubleBuffer(m_DoubleBuffer);

	}
	m_renderParam->SetSplitScreenNum(m_SplitScreen);
	m_renderParam->SetCardMaskID(m_CardMask);
	if (m_rectDraw == NULL)
	{
		m_rectDraw = new RectDraw();
		m_rectDraw->init(1, 1);
		m_InputSRV = GetDynamicRHI()->CreateTexture();
	}
	m_InputSRV->AttatchSRV(pInputTexture);
	m_rectDraw->setShaderTextureView(m_InputSRV);
	m_DoubleBuffer->BindFBOA();

	if (needToClearBuffer)
	{
		m_DoubleBuffer->GetFBOA()->clear(0, 0, 0, 0);
	}

	if (needToCopyInputTexture)
	{
		m_rectDraw->renderOpaque();
	}
	m_renderParam->SetFaceNum(0);

	m_DoubleBuffer->m_rectDraw = m_rectDraw;
	if (faceRes != NULL)
	{
		m_renderParam->SetFaceNum(faceRes->numFace);
		for (int n = 0; n < faceRes->numFace; n++)
		{
			m_renderParam->SetFacePoint130((Vector2*)faceRes->arrFace[n].arrShapeDense, n, width, height);
			FacePosInfo pFaceInfo;
			memcpy(pFaceInfo.arrWMatrix, faceRes->arrFace[n].arrWMatrix, 16 * sizeof(float));
			pFaceInfo.pitch = faceRes->arrFace[n].pitch;
			pFaceInfo.roll = faceRes->arrFace[n].roll;
			pFaceInfo.yaw = faceRes->arrFace[n].yaw;
			pFaceInfo.x = faceRes->arrFace[n].x;
			pFaceInfo.y = faceRes->arrFace[n].y;
			pFaceInfo.z = faceRes->arrFace[n].z;
			pFaceInfo.faceW = faceRes->arrFace[n].faceRect.right - faceRes->arrFace[n].faceRect.left;
			pFaceInfo.faceH = faceRes->arrFace[n].faceRect.bottom - faceRes->arrFace[n].faceRect.top;
			
			pFaceInfo.pFaceRect.x = faceRes->arrFace[n].gesture_box_left;
			pFaceInfo.pFaceRect.y = faceRes->arrFace[n].gesture_box_top;
			pFaceInfo.pFaceRect.width = faceRes->arrFace[n].gesture_box_right - faceRes->arrFace[n].gesture_box_left;
			pFaceInfo.pFaceRect.height = faceRes->arrFace[n].gesture_box_bottom - faceRes->arrFace[n].gesture_box_top;

			pFaceInfo.pFaceExp = (FaceExpression *)(&faceRes->arrFace[n].faceExpression);

			m_renderParam->SetFacePosInfo(&pFaceInfo, n);
		}
	}
	if (m_pTextureMask != NULL)
	{
		m_renderParam->SetBodyMaskTexture(m_pTextureMask);
	}
	if (m_pTextureHairMask != NULL)
	{
		m_renderParam->SetHairMaskTexture(m_pTextureHairMask);

	}
	if (m_pHandRes != NULL)
	{
		m_renderParam->SetHandInfo(m_pHandRes);
	}
	if (m_pBodyRes != NULL)
	{
		m_renderParam->SetBodyPoint(m_pBodyRes);
	}


	for (int i = 0; i < m_AllEffectPart.size(); i++)
	{
		if (m_AllEffectPart[i]->m_bEnableReder)
		{
			m_AllEffectPart[i]->m_runTime = runTime;
			m_AllEffectPart[i]->Render(*m_renderParam);
		}

	}
}

void CCEffectInterface::loadEffectFromZip_asyn_impl(const char * szPath, const char *szXMLFile)
{
	if (m_hLoadThread != 0)
	{
		CloseHandle(m_hLoadThread);
		m_hLoadThread = 0;
	}
	m_resourceAsyn.m_AllEffectPart.clear();
	m_resourceAsyn.szPath = szPath;
	m_resourceAsyn.szXMLFile = szXMLFile;
	m_resourceAsyn.m_resourcePath = m_resourcePath;
	m_hLoadThread = CreateThread(NULL, 0, ReadConfigThread, &m_resourceAsyn, 0/*CREATE_SUSPENDED*/, NULL);

	m_effectStatus = CES_LOADING;
}

void CCEffectInterface::loadEffectFromZip_sync_impl(const char * szPath, const char *szXMLFile)
{
	m_resourceAsyn.m_AllEffectPart.clear();
	m_resourceAsyn.szPath = szPath;
	m_resourceAsyn.szXMLFile = szXMLFile;
	m_resourceAsyn.m_resourcePath = m_resourcePath;
	ReadConfigThread(&m_resourceAsyn);
}

void CCEffectInterface::touchThreadSource()
{
	if (m_resourceAsyn.status == CES_READY)
	{
		if (m_hLoadThread != 0)
		{
			WaitForSingleObject(m_hLoadThread, 10000);
			CloseHandle(m_hLoadThread);
			m_hLoadThread = 0;
		}
		RealeseEffectPart();
		m_AllEffectPart = m_resourceAsyn.m_AllEffectPart;
		m_effectStatus = CES_IDLE;
		m_resourceAsyn.reset();
		m_nStartTime = -1;
		if (m_LastZip !="")
		{
			RealeseEffectPart();
			loadEffectFromZipAsyn(m_LastZip, "test.xml");
			m_LastZip = "";
		}
		if (m_AllEffectPart.size()>0)
		{
			m_effectStatus = CES_RUN;
			if (m_pEffectCallback != NULL)
			{
				(*m_pEffectCallback)(EE_START, m_lpParam);
			}
		}

	}


	for (int i = 0; i < m_AllEffectPart.size(); i++)
	{
		if (m_AllEffectPart[i]->m_MSAA)
		{
			m_MSAA = true;
			break;
		}
	}

}

bool CCEffectInterface::loadEffectFromZipAsyn(const std::string& szZipFile, const std::string& xml, EffectCallback callback, void * lpParam)
{

	if (szZipFile.size() > 0)
	{
		if (m_effectStatus == CES_LOADING)
		{
			m_LastZip = szZipFile;
			return true;
		}
		m_pEffectCallback = callback;
		m_lpParam = lpParam;
		loadEffectFromZip_asyn_impl(szZipFile.c_str(),xml.c_str());
		touchThreadSource();
	}
	else
	{
		return false;
	}
	return true;
}

bool CCEffectInterface::loadEffectFromZipSync(const std::string& szZipFile, const std::string& xml, EffectCallback callback, void * lpParam)
{
	if (m_effectStatus == CES_LOADING)
	{
		if (m_hLoadThread != 0)
		{
			WaitForSingleObject(m_hLoadThread, 10000);
			CloseHandle(m_hLoadThread);
			m_hLoadThread = 0;
		}
		return false;
	}
	if (szZipFile.size() > 0)
	{
		m_pEffectCallback = callback;
		m_lpParam = lpParam;
		loadEffectFromZip_sync_impl(szZipFile.c_str(), xml.c_str());
		touchThreadSource();
	}
	else
	{
		RealeseEffectPart();
		m_effectStatus = CES_IDLE;

	}
	return true;
}

bool CCEffectInterface::AddEffectFromXML(const std::string& dir, const std::string& xml)
{
	string szXml = dir + "/" + xml;
	XMLResults xResults;
	XMLNode nodeModels = XMLNode::parseFile(szXml.c_str(), "models", &xResults);
	int i = -1;
	XMLNode nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
	while (!nodeEffect.isEmpty())
	{
		const char *szType = nodeEffect.getAttribute("type");
		auto pPart = GetPart(szType);
		pPart->SetResourcePath(m_resourcePath);
		pPart->ReadConfig(nodeEffect, dir);
		pPart->Prepare();
		m_AllEffectPart.push_back(pPart);
		nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
	}
	m_resourceAsyn.m_nEffectDuring = 99999999999;
	m_resourceAsyn.m_resourcePath = m_resourcePath;
	m_nStartTime = -1;
	m_effectStatus = CES_RUN;

	return true;
}

bool CCEffectInterface::renderEffectToTexture(ID3D11ShaderResourceView * pInputTexture, ID3D11Texture2D * pTargetTexture, int width, int height, const ccFDFaceRes * faceRes)
{
	return renderEffectToTexture(pInputTexture, pTargetTexture, width, height, faceRes, true,false);
}

bool CCEffectInterface::renderEffectToTexture(ID3D11ShaderResourceView* pInputTexture, ID3D11Texture2D* pTargetTexture, int width, int height, const ccFDFaceRes* faceRes, bool needToCopyInputTexture,bool needToClearBuffer)
{
	touchThreadSource();
	if (m_effectStatus != CES_RUN)
	{
		return false;
	}

	if (m_AllEffectPart.size() == 0)
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	pTargetTexture->GetDesc(&texDesc);

	m_format = texDesc.Format;
	renderEffect(pInputTexture, width, height, faceRes, needToCopyInputTexture, needToClearBuffer);
	unsigned int sub = D3D11CalcSubresource(0, 0, 1);
	DeviceContextPtr->ResolveSubresource(
		pTargetTexture,
		sub,
		RHIResourceCast(m_DoubleBuffer->GetFBOTextureA().get())->GetNativeTex(),
		sub,
		m_format
	);


	return (m_effectStatus == CES_RUN);
}


ID3D11ShaderResourceView * CCEffectInterface::renderEffectToTexture(ID3D11ShaderResourceView * pInputTexture, int width, int height, const ccFDFaceRes * faceRes)
{

	touchThreadSource();
	if (m_effectStatus != CES_RUN)
	{
		return nullptr;
	}

	if (m_AllEffectPart.size() == 0)
	{
		return nullptr;
	}
	renderEffect(pInputTexture, width, height, faceRes,true,false);

	return RHIResourceCast(m_DoubleBuffer->GetFBOTextureA().get())->GetSRV() ;
}

void CCEffectInterface::SetMask(unsigned char * pMask, int nWidth, int nHeight,CCEffectType type, AnchorType anchortype)
{
	//传主播的头像和ID
	if (anchortype != ANCHOR_UNKNOW)
	{
		long long cardId = ResourceManager::Instance().loadImage(pMask, nWidth, nHeight);
		m_CardMask[anchortype] = cardId;
	}

	if (type == LUT_FILTER_EFFECT)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlter *part = (CLUTFIlter  *)m_AllEffectPart[i];
				ResourceManager::Instance().getAnimFrame(part->m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
				break;
			}
		}
	}
	else if (type == TYPE_FILTER_EFFECT)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == TYPE_FILTER_EFFECT)
			{
				CLUTFIlter *part = (CLUTFIlter  *)m_AllEffectPart[i];
				ResourceManager::Instance().getAnimFrame(part->m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
				if (part->m_alpha <0.01)
				{
					part->m_alpha = 1.0;
				}
				break;
			}
		}
	}

	else if (type == FACE_EYESTRICK )
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceBlendMakeUp *part = (CFaceBlendMakeUp  *)m_AllEffectPart[i];
				for (int j=0;j<part->m_vMeshType.size();j++)
				{
					if (part->m_vMeshType[j].m_meshType == MESH_EYE)
					{
						ResourceManager::Instance().getAnimFrame(part->m_vMeshType[j].m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						//BitmapDrawable * m_drawable = (BitmapDrawable *)part->m_vMeshType[j].m_drawable;
						//m_drawable->m_pTex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						break;
					}
				}
				break;
			}
		}
	}

	else if (type == FACE_EYEBROWSTRICK)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceBlendMakeUp *part = (CFaceBlendMakeUp  *)m_AllEffectPart[i];
				for (int j = 0; j < part->m_vMeshType.size(); j++)
				{
					if (part->m_vMeshType[j].m_meshType == MESH_EYEBROW)
					{
						//BitmapDrawable * m_drawable = (BitmapDrawable *)part->m_vMeshType[j].m_drawable;
						//m_drawable->m_pTex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						ResourceManager::Instance().getAnimFrame(part->m_vMeshType[j].m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						break;
					}
					
				}
				break;
			}
		}
	}

	else if (type == FACE_MOUTH_STRICK)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceBlendMakeUp *part = (CFaceBlendMakeUp  *)m_AllEffectPart[i];
				for (int j = 0; j < part->m_vMeshType.size(); j++)
				{
					if (part->m_vMeshType[j].m_meshType == MESH_MOUTH)
					{
						//BitmapDrawable * m_drawable = (BitmapDrawable *)part->m_vMeshType[j].m_drawable;
						//m_drawable->m_pTex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						ResourceManager::Instance().getAnimFrame(part->m_vMeshType[j].m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						break;
					}

				}
				break;
			}
		}
	}
	else if (type == FACE_EYELID)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceBlendMakeUp *part = (CFaceBlendMakeUp  *)m_AllEffectPart[i];
				for (int j = 0; j < part->m_vMeshType.size(); j++)
				{
					if (part->m_vMeshType[j].m_meshType == MESH_EYE_LID)
					{
						//BitmapDrawable * m_drawable = (BitmapDrawable *)part->m_vMeshType[j].m_drawable;
						//m_drawable->m_pTex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						ResourceManager::Instance().getAnimFrame(part->m_vMeshType[j].m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
						break;
					}

				}
				break;
			}
		}
	}

	else if (type == FACE_BLUSH)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_BLUSH)
			{
				CFaceEffect2DBlendBG *part = (CFaceEffect2DBlendBG  *)m_AllEffectPart[i];
				ResourceManager::Instance().getAnimFrame(part->m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
				break;
			}
		}
	}

	else if (type == FACE_EFFECT_STICKER)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_EFFECT_STICKER)
			{
				CFaceEffect2DSticker *part = (CFaceEffect2DSticker  *)m_AllEffectPart[i];				
				ResourceManager::Instance().getAnimFrame(part->m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
				break;
			}
		}
	}

	else if(type == FACE_3D_SWITCH_PALATA  )
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++)
		{
			if (m_AllEffectPart[i]->m_EffectPart == FACE_PBR_3D_MODEL)
			{
				CFacePBRModel *part = (CFacePBRModel*)(m_AllEffectPart[i]);
				if (part->m_RenderUtils->m_3DScene != NULL && !part->m_RenderUtils->m_3DScene->HasLoadModel)
				{
					CC3DModel *pModel = part->m_RenderUtils->m_3DScene->m_Model[0];
					int nMaterial = pModel->m_ModelMaterial.size();
					for (int j = 0; j < nMaterial; j++)
					{
						if (pModel->m_ModelMaterial[j]->MaterialName == part->m_RenderUtils->m_SwitchPalate)
						{
							pModel->m_ModelMaterial[j]->m_BaseColorTexture = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, nWidth, nHeight, pMask, 4 * nWidth);
							//pModel->m_ModelMaterial[j]->m_BaseColorTexture->LoadTexture(pMask, nWidth, nHeight);
							break;
						}
					}
				}
				break;
			}

		}
	}
	else
	{
	  //  //临时处理下
	  //  if ( (type == UNKNOW_EFFECT && m_AllEffectPart.size() == 1 && m_AllEffectPart[0]->m_EffectPart == FACE_PBR_3D_MODEL))
	  //  {
			//int nPart = (int)m_AllEffectPart.size();
			//for (int i = 0; i < nPart; i++)
			//{
			//	if (m_AllEffectPart[i]->m_EffectPart == FACE_PBR_3D_MODEL)
			//	{
			//		CFacePBRModel *part = (CFacePBRModel*)(m_AllEffectPart[i]);
			//		if (part->m_RenderUtils->m_3DScene != NULL && !part->m_RenderUtils->m_3DScene->m_hasLoadModel)
			//		{
			//			CC3DModel *pModel = part->m_RenderUtils->m_3DScene->m_Model[0];
			//			int nMaterial = pModel->m_ModelMaterial.size();
			//			for (int j = 0; j < nMaterial; j++)
			//			{
			//				if (!pModel->m_ModelMaterial[j]->m_BaseColorTexture || (pModel->m_ModelMaterial[j]->MaterialName == part->m_RenderUtils->m_SwitchPalate &&pModel->m_ModelMaterial[j]->m_BaseColorTexture->GetWidth() != nWidth))
			//				{
			//					pModel->m_ModelMaterial[j]->m_BaseColorTexture = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, nWidth, nHeight, pMask,4*nWidth);
			//					//pModel->m_ModelMaterial[j]->m_BaseColorTexture->LoadTexture(pMask, nWidth, nHeight);
			//					break;
			//				}
			//			}
			//		}
			//		break;
			//	}

			//}
	  //  }
		//临时处理下2
		//if ((type == UNKNOW_EFFECT && m_AllEffectPart.size() == 2 && m_AllEffectPart[1]->m_EffectPart == FACE_EFFECT_STICKER))
		//{
		//	CFaceEffect2DSticker *part = (CFaceEffect2DSticker*)(m_AllEffectPart[1]);
		//	ResourceManager::Instance().getAnimFrame(part->m_anim_id, 0)->tex->updateTextureInfo(pMask, nWidth * 4, nHeight);
		//}

		int maskW = nWidth;
		int maskH = nHeight;
		if (!m_pTextureMask)
		{
			m_pTextureMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, CC3DTextureRHI::OT_NONE, maskW, maskH, pMask, maskW, false);
		}
		else if (m_pTextureMask->GetWidth() == maskW && m_pTextureMask->GetHeight() == maskH)
		{
			m_pTextureMask->updateTextureInfo(pMask, maskW, maskH);
		}
		else
		{
			m_pTextureMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, CC3DTextureRHI::OT_NONE, maskW, maskH, pMask, maskW, false);
		}
	}


}



void CCEffectInterface::SetHairMask(unsigned char * pMask, int nWidth, int nHeight, CCEffectType type)
{
	int maskW = nWidth;
	int maskH = nHeight;
	if (m_pTextureHairMask == NULL)
	{
		m_pTextureHairMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, CC3DTextureRHI::OT_NONE, maskW, maskH, pMask, maskW, false);
		//m_pTextureHairMask->initTexture(DXGI_FORMAT_R8_UNORM, D3D11_BIND_SHADER_RESOURCE, maskW, maskH, pMask, maskW, false);
	}
	else if (m_pTextureHairMask->GetWidth() == maskW && m_pTextureHairMask->GetHeight() == maskH)
	{
		m_pTextureHairMask->updateTextureInfo(pMask, maskW, maskH);
	}
	else
	{
		m_pTextureHairMask = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R8, CC3DTextureRHI::OT_NONE, maskW, maskH, pMask, maskW, false);
		//m_pTextureHairMask->destory();
		//m_pTextureHairMask->initTexture(DXGI_FORMAT_R8_UNORM, D3D11_BIND_SHADER_RESOURCE, maskW, maskH, pMask, maskW, false);
	}
}

void CCEffectInterface::SetHand(ccHandRes* handRes)
{
	if (handRes != NULL)
	{
		m_pHandRes = handRes;
	}
}

void CCEffectInterface::SetBodyPoint(ccBodyRes * bodyRes)
{
	if (bodyRes != NULL)
	{
		m_pBodyRes = bodyRes;
	}
}

void CCEffectInterface::SetBGRA(unsigned char * pRGBA)
{
	if (m_renderParam!=NULL)
	{
		m_renderParam->m_pBGRA_Src = pRGBA;
	}
}

void CCEffectInterface::SetRotate(float x, float y, float z, CCEffectType type)
{
}


void CCEffectInterface::SetColor(float r, float g, float b, float a, CCEffectType type)
{
	if (type == FACE_LIPSTRICK)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_LIPSTRICK)
			{
				CFaceLipstick *part = (CFaceLipstick  *)m_AllEffectPart[i];
				part->m_MouthRGBA = Vector4(r, g, b, a) / 255.f;
				break;
			}
		}
	}
	if (type == FlipSplit)
	{
		int nPart = (int)m_AllEffectPart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FlipSplit)
			{
				CFaceEffectFlipSplit* part = (CFaceEffectFlipSplit*)m_AllEffectPart[i];
				part->mControlParam = Vector4(r, g, b, a);
				break;
			}
		}
	}

}

void CCEffectInterface::setAlpha(float Alpha, CCEffectType type)
{
	int nPart = (int)m_AllEffectPart.size();
	for (int i = 0; i < nPart; i++) {
		if (m_AllEffectPart[i]->m_EffectPart == type)
		{
			m_AllEffectPart[i]->m_alpha = Alpha;
			break;
		}
	}

	if (type == LUT_FILTER_EFFECT)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlter *part = (CLUTFIlter *)m_AllEffectPart[i];
				part->m_alpha = Alpha;
				break;
			}
		}
	}
	if (type>= FACE_LIFT_EFFECT && type < FACE_LIFT_EFFECT+100)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_LIFT_EFFECT)
			{
				CFaceBeauty *part = (CFaceBeauty *)m_AllEffectPart[i];
				part->SetAlpha(Alpha, type);
				break;
			}
		}
	}
	if (type == FACE_EYESTRICK || type == FACE_EYEBROWSTRICK || type == FACE_MOUTH_STRICK || type == FACE_EYELID)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceBlendMakeUp *part = (CFaceBlendMakeUp *)m_AllEffectPart[i];
				for (int j = 0; j < part->m_vMeshType.size(); j++)
				{
					if ((part->m_vMeshType[j].m_meshType == MESH_EYE && type == FACE_EYESTRICK) || (part->m_vMeshType[j].m_meshType == MESH_EYE_LID && type == FACE_EYELID) || (part->m_vMeshType[j].m_meshType == MESH_EYEBROW && type == FACE_EYEBROWSTRICK) || (part->m_vMeshType[j].m_meshType == MESH_MOUTH && type == FACE_MOUTH_STRICK))
					{
						part->m_vMeshType[j].alpha = Alpha;
						break;
					}

				}
				break;
			}
		}
	}

	else if (type == FACE_WHITE_TEETH)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_LIPSTRICK)
			{
				CFaceLipstick *part = (CFaceLipstick *)m_AllEffectPart[i];
				part->m_wihteTeethAlpha = Alpha;
				break;
			}
		}
	}
	else if (type == FACE_REMOVE_POUCH || type == FACE_BRIGHT_EYE)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_AllEffectPart[i]->m_EffectPart == FACE_REMOVE_FALIN)
			{
				CFaceRemovePouchFalin* part = (CFaceRemovePouchFalin*)m_AllEffectPart[i];
				if (type == FACE_REMOVE_POUCH)
				{
					part->m_pouchAlpha = Alpha;
				}
				else
				{
					part->m_BrightEyeAlpha = Alpha;
				}

				break;
			}
		}
	}
}

void CCEffectInterface::SetSplitScreen(int SplitScreen, CCEffectType type)
{
	m_SplitScreen = SplitScreen;
}

void CCEffectInterface::SetBlendShapeParam(float * pBlendShapeParam)
{
}

void CCEffectInterface::SetResourcePath(const char *path)
{
	m_resourcePath = path;
	CC3DEnvironmentConfig::getInstance()->resourth_path = m_resourcePath;
}

void CCEffectInterface::zipAllResource(std::string &zipPath, std::string &tempPath, long duringtime, const char* version) {
	//resetTempFolder();

	HZIP dst = CreateZip(zipPath.c_str(), nullptr);

	writeAllResource(tempPath);

	std::string xml_file_name = tempPath + "/" + "giftDesc.xml";
	XMLNode timeRoot = XMLNode::createXMLTopNode("giftDesc");
	timeRoot.addAttribute("version", version);
	timeRoot.addAttribute("effectXML", "test.xml");

	char time[128];
	sprintf(time, "%d", duringtime);
	timeRoot.addAttribute("during", time);
	timeRoot.writeToFile(xml_file_name.c_str());

	zipFolder(dst, tempPath);
	CloseZip(dst);

	//removeFolder(tempPath);
}

void CCEffectInterface::ZipAllResourceOnly(const std::string& ZipPath, const std::string& ContanFolder)
{

	HZIP dst = CreateZip(ZipPath.c_str(), nullptr);
	zipFolder(dst, ContanFolder);

	CloseZip(dst);
}

void CCEffectInterface::writeAllResource(std::string &tempPath, const char* version)
{

	XMLNode root = XMLNode::createXMLTopNode("models");
	root.addAttribute("version", version);

	for (int i = 0; i < m_AllEffectPart.size(); i++)
	{
		if (m_AllEffectPart[i]->m_bEnableWrite && m_AllEffectPart[i]->m_bEnableReder)
		{
			m_AllEffectPart[i]->WriteConfig(tempPath, root);
		}	
	}
	std::string xml_file_name = tempPath + "/" + "test.xml";
	root.writeToFile(xml_file_name.c_str());
}

void CCEffectInterface::WriteAllXMLConfig(const std::string& Path, const std::string& Name, const std::string& Version /*= "1.2"*/)
{
	XMLNode root = XMLNode::createXMLTopNode("models");
	root.addAttribute("version", Version.c_str());

	for (int i = 0; i < m_AllEffectPart.size(); i++)
	{
		if (m_AllEffectPart[i]->m_bEnableWrite && m_AllEffectPart[i]->m_bEnableReder)
		{
			m_AllEffectPart[i]->WriteConfig(const_cast<std::string&>(Path), root);
		}
	}
	std::string xml_file_name = Path + Name;
	root.writeToFile(xml_file_name.c_str());
}

void CCEffectInterface::addEffect(CEffectPart * part)
{
	if (part) m_AllEffectPart.push_back(part);
}

void CCEffectInterface::deleteEffect(CEffectPart* part) {
	auto part_res = std::find(m_AllEffectPart.begin(), m_AllEffectPart.end(), part);
	if (part_res != m_AllEffectPart.end()) {
		delete part;
		m_AllEffectPart.erase(part_res);
	}
}

bool CCEffectInterface::exchangePart(int src_index,int dst_index) {
	if(src_index >= 0 && src_index < m_AllEffectPart.size() && dst_index >= 0 && dst_index < m_AllEffectPart.size()) {
		CEffectPart* src_part = m_AllEffectPart[src_index];
		CEffectPart* dst_part = m_AllEffectPart[dst_index];
		m_AllEffectPart[src_index] = dst_part;
		m_AllEffectPart[dst_index] = src_part;
		return true;
	}
	return false;
}

bool CCEffectInterface::MovePart(int src_index, int dst_index)
{

	if (src_index >= 0 && src_index < m_AllEffectPart.size() && dst_index >= 0 && dst_index < m_AllEffectPart.size()) {
		auto it = m_AllEffectPart.begin() + src_index;
		auto it2 = m_AllEffectPart.begin() + dst_index;
		CEffectPart *part = *it;
		m_AllEffectPart.erase(it);
		m_AllEffectPart.insert(it2, part);
		return true;
	}
	return false;
}


void CCEffectInterface::SetMakeUpZip(const string & szZipFile)
{
	std::wstring wPath = core::u8_ucs2(szZipFile);

	std::ifstream inStream(wPath, ios_base::in | ios_base::binary);
	std::stringstream buffer;
	buffer << inStream.rdbuf();
	std::string contents(buffer.str());

	HZIP hZip = nullptr;
	if (contents.empty())
	{
		hZip = OpenZip(szZipFile.c_str(), nullptr);
	}
	else
	{
		hZip = OpenZip((char*)contents.data(), contents.length(), nullptr);
	}

	if (hZip != NULL)
	{
		ZIPENTRY ze;
		int index;
		if (ZR_OK == FindZipItem(hZip, "test.xml", true, &index, &ze))
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res != ZR_OK)
			{
				delete[]pDataBuffer;
				CloseZip(hZip);
				return ;
			}
			XMLResults xResults;
			XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
			int ii = -1;
			XMLNode nodeEffect = nodeModels.getChildNode("typeeffect", ++ii);
			bool HasEye = false;
			while (!nodeEffect.isEmpty())
			{
				const char *szType = nodeEffect.getAttribute("type");
				
				if (szType != NULL && (!strcmp(szType, "MakeUpBlend")))
				{
					int nPart = (int)m_AllEffectPart.size();
					CFaceBlendMakeUp *part = NULL;
					for (int i = 0; i < nPart; i++) {
						if (m_AllEffectPart[i]->m_EffectPart == FACE_EYESTRICK)
						{
							part  = (CFaceBlendMakeUp  *)m_AllEffectPart[i];
						}
					}
					if (part == NULL)
					{
						nodeEffect = nodeModels.getChildNode("typeeffect", ++ii);
						continue;
					}
					XMLNode nodeDrawable = nodeEffect.getChildNode("anidrawable",0);
					if (!nodeDrawable.isEmpty())
					{
						const char *szType = nodeDrawable.getAttribute("FaceType");

						const char *szBlendType = nodeDrawable.getAttribute("blendType");
						auto m_blendType = part->GetBlendType(szBlendType);
						FaceMeshType m_meshType;

						if (szType != NULL && !strcmp(szType, "Eyebrow"))
						{
							m_meshType = MESH_EYEBROW;

						}
						else if (szType != NULL && !strcmp(szType, "Eye"))
						{
							if (HasEye)
							{
								m_meshType = MESH_EYE_LID;
								part->m_IsEyelidVisable = true;
							}
							else
							{
								HasEye = true;
								m_meshType = MESH_EYE;
								part->m_IsEyelidVisable = false;
							}
							
						}
						else if (szType != NULL && !strcmp(szType, "EyeMult"))
						{
							m_meshType = MESH_EYE_LID;
							part->m_IsEyelidVisable = true;
						}
						else if (szType != NULL && !strcmp(szType, "Mouth"))
						{
							m_meshType = MESH_MOUTH;
							SetColor(0,0,0,0);
						}
						
						const char *szItems = nodeDrawable.getAttribute("items");
						const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
						char szImagePath[128];
						int iStart = 0;
						int nCount = 0;
						int nDuring = 0;
						int nStep = 1;
						sscanf(szItemInfo, "%d,%d,%d,%d", &iStart, &nCount, &nDuring, &nStep);
						sprintf(szImagePath, szItems, iStart);
						int w, h;
						BYTE *pData = DXUtils::CreateImgFromZIP(hZip,szImagePath, w, h);

						for (int j = 0; j < part->m_vMeshType.size(); j++)
						{
							if (part->m_vMeshType[j].m_meshType == m_meshType)
							{
								part->m_vMeshType[j].m_blendType = m_blendType;
								ResourceManager::Instance().getAnimFrame(part->m_vMeshType[j].m_anim_id, 0)->tex->updateTextureInfo(pData, w * 4, h);
								break;
							}

						}
						SAFE_DELETE_ARRAY(pData);
					}
				}

				else if (szType != NULL && (!strcmp(szType, "FaceBlush")))
				{
					int nPart = (int)m_AllEffectPart.size();
					CFaceEffect2DBlendBG *part = NULL;
					for (int i = 0; i < nPart; i++) {
						if (m_AllEffectPart[i]->m_EffectPart == FACE_BLUSH)
						{
							part = (CFaceEffect2DBlendBG  *)m_AllEffectPart[i];
						}
					}
					if (part == NULL)
					{
						nodeEffect = nodeModels.getChildNode("typeeffect", ++ii);
						continue;
					}
					XMLNode nodeDrawable = nodeEffect.getChildNode("anidrawable", 0);
					if (!nodeDrawable.isEmpty())
					{
						const char *szType = nodeDrawable.getAttribute("FaceType");

						const char *szBlendType = nodeDrawable.getAttribute("blendType");
						auto m_blendType = part->GetBlendType(szBlendType);
	
						const char *szItems = nodeDrawable.getAttribute("items");
						const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
						char szImagePath[128];
						int iStart = 0;
						int nCount = 0;
						int nDuring = 0;
						int nStep = 1;
						sscanf(szItemInfo, "%d,%d,%d,%d", &iStart, &nCount, &nDuring, &nStep);
						sprintf(szImagePath, szItems, iStart);
						int w, h;
						BYTE *pData = DXUtils::CreateImgFromZIP(hZip,szImagePath, w, h);
						part->m_BlendType = m_blendType;
						ResourceManager::Instance().getAnimFrame(part->m_anim_id, 0)->tex->updateTextureInfo(pData, w * 4, h);
						SAFE_DELETE_ARRAY(pData);
					}
				}

				else if (szType != NULL && (!strcmp(szType, "FaceLipstick")))
				{
					XMLNode node = nodeEffect.getChildNode("param", 0);
					if (!node.isEmpty())
					{
						const char *szMouthColor = node.getAttribute("mouthColor");
						Vector4 m_MouthRGBA;
						if (szMouthColor != NULL)
						{
							sscanf(szMouthColor, "%f,%f,%f,%f", &m_MouthRGBA.x, &m_MouthRGBA.y, &m_MouthRGBA.z, &m_MouthRGBA.w);
						}
						SetColor(m_MouthRGBA.x, m_MouthRGBA.y, m_MouthRGBA.z, m_MouthRGBA.w);
					}
				}


				nodeEffect = nodeModels.getChildNode("typeeffect", ++ii);
			}

		}
	}
	CloseZip(hZip);
}

void CCEffectInterface::UpdateTestXmlWithNewAttr(XMLNode& TestXMLNode)
{
	TestXMLNode = ResourceManager::Instance().CurrentTestXMLInZip;
	int i = -1;
	XMLNode nodeEffect;
	while (!(nodeEffect = TestXMLNode.getChildNode("typeeffect", ++i)).isEmpty())
	{
		XMLNode nodeanidrawable = nodeEffect.getChildNode("anidrawable");
		if (nodeanidrawable.isEmpty())continue;
		const char* szShowname = nodeEffect.getAttribute("showname");
		const char* szAlpha = nodeanidrawable.getAttribute("alpha");
		if (szAlpha != NULL && szShowname != NULL)
		{
			CEffectPart* pEffect = nullptr;
			for (int i = 0; i < m_AllEffectPart.size(); i++)
			{
				if (m_AllEffectPart[i]->m_showname == std::string(szShowname))
				{
					pEffect = m_AllEffectPart[i];
					break;
				}
			}
			if (pEffect)
			{
				char AttrStr[128] = {};
				sprintf(AttrStr, "%f", pEffect->m_alpha);
				nodeanidrawable.deleteAttribute("alpha");
				nodeanidrawable.addAttribute("alpha", AttrStr);
			}
		}
	}
}