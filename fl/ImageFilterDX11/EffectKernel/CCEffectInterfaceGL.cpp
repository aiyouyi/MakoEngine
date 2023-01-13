#include "CCEffectInterfaceGL.h"
#include "Toolbox/zip/unzip.h"
#include "Toolbox/zip/zip.h"
#include "BaseRenderParam.h"
#include "CEffectPart.h"
#include "Toolbox/GL/DoubleBuffer.h"
#include "BaseDefine/commonFunc.h"
#include "EffectKernel/FaceEffectGL/CFaceEffect2DGL.h"
#include "EffectKernel/FaceEffectGL/CFaceEffect2DWithBGGL.h"
#include "EffectKernel/FaceEffectGL/CFaceEffect2DBlendBGGL.h"
#include "EffectKernel/FaceEffectGL/CFaceEffectMakeUp.h"
#include "EffectKernel/FaceEffectGL/CFaceEffectBlendMakeUp.h"
#include "EffectKernel/FaceEffectGL/CFaceEffect3DModelGL.h"
#include "EffectKernel/FaceLiftGL/CFaceMeshChangeGL.h"
#include "EffectKernel/FaceLiftGL/CFaceSPMLSGL.h"
#include "EffectKernel/FaceEffectGL/CFaceSmoothInsGL.h"
#include "EffectKernel/FilterGL/CLUTFIlterGL.h"
#include "EffectKernel/FilterGL/CFoodieSharpGL.h"
#include "EffectKernel/FaceEffectGL/CFaceShadowHighLightGL.h"
#include "EffectKernel/FilterGL/CDenosieFilterGL.h"
#include "EffectKernel/StickerGL/CStickerEffectGL.h"
#include "EffectKernel/FaceLiftGL/CFaceBeautyGL.h"
#include "EffectKernel/FaceEffectGL/CFaceLipstickGL.h"
#include "EffectKernel/FaceEffectGL/CFaceLevelGL.h"
#include "EffectKernel/FaceEffectGL/CFaceRemovePouchFalinGL.h"
#include "EffectKernel/StickerGL/CFaceEffect2DStickerGL.h"

//#include "EffectKernel/PBR/CWallPaperPBRModel.h"
#include "EffectKernel/PBRGL/CFacePBRModelGL.h"

#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "ToolBox/RenderState/PiplelineState.h"
#include "CC3DEngine/RenderGL/ShaderProgramManagerGL.h"
#include "ToolBox/GL/GLDynamicRHI.h"
#include "Toolbox/GL/GLResource.h"

#include<thread>
#ifdef PLATFORM_ANDROID
#include <android/log.h>
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif
//#include "opencv/cv.h"

CCEffectInterfaceGL::CCEffectInterfaceGL()
{
	m_lpParam = NULL;
	m_pEffectCallback = NULL;

	m_DoubleBuffer = NULL;
    m_renderParam = NULL;

	m_resourcePath = "./";

	CC3DPiplelineState::Initialize();
	m_effectStatus = CES_IDLE;
}


CCEffectInterfaceGL::~CCEffectInterfaceGL()
{
	Realese();
	CC3DPiplelineState::DestroyAll();
	CC3DEnvironmentConfig::Release();
	ShaderProgramManagerGL::GetInstance()->Release();
	ShaderProgramManagerGL::ReleaseInstance();
	GLDynamicRHI::Release();
	
}


void CCEffectInterfaceGL::Realese()
{
	//m_resourceAsyn.Release(); //it was released by RealeseEffectPart();
    RealeseBasePart();
	m_DoubleBuffer.reset();
    SAFE_DELETE(m_renderParam);
    touchThreadSource();
    RealeseEffectPart();

}

void CCEffectInterfaceGL::setAlpha(float Alpha,CCEffectType type)
{
	int nPart = (int)m_BasePart.size();
	
	for (int i = 0; i < nPart; i++) {
		if (m_BasePart[i]->m_EffectPart == type)
		{
			m_BasePart[i]->m_alpha = Alpha;
			break;
		}
	}

	if (type == LUT_ADJUST_EFFECT || type == LUT_CLEAR_EFFECT || TYPE_FILTER_EFFECT)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlterGL *part = (CLUTFIlterGL *)m_BasePart[i];
				if (type == LUT_ADJUST_EFFECT)
				{
					part->m_alphaAdjust = Alpha;
				}
				else if(type == LUT_CLEAR_EFFECT)
				{
					part->m_alphaClear = Alpha;
				}
				else if (type == TYPE_FILTER_EFFECT)
				{
					part->m_alphaType = Alpha;
				}
				break;
			}
		}
	}

	//眼影 双眼皮 眉毛 嘴巴
	if (type == FACE_EYESTRICK || type == FACE_EYEBROWSTRICK || type == FACE_MOUTH_STRICK || type == FACE_EYELID)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{

					if ((part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE && type == FACE_EYESTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE_LID && type == FACE_EYELID) || 
						(part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYEBROW && type == FACE_EYEBROWSTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_MOUTH && type == FACE_MOUTH_STRICK))
					{
						part->m_vMakeUpBlendInfo[j].alpha = Alpha;
						break;
					}

				}
				break;
			}
		}
	}

	if (type >= FACE_LIFT_EFFECT && type < FACE_LIFT_EFFECT+100)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_LIFT_EFFECT)
			{
				CFaceBeautyGL *part = (CFaceBeautyGL *)m_BasePart[i];
				part->SetAlpha(Alpha, type);
				break;
			}
		}
	}
	else if (type == FACE_WHITE_TEETH)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_LIPSTRICK)
			{
				CFaceLipstickGL *part = (CFaceLipstickGL *)m_BasePart[i];
				part->m_wihteTeethAlpha = Alpha;
				break;
			}
		}
	}
	else if (type == FACE_REMOVE_POUCH || type == FACE_BRIGHT_EYE)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_REMOVE_FALIN)
			{
				CFaceRemovePouchFalinGL* part = (CFaceRemovePouchFalinGL*)m_BasePart[i];
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

float CCEffectInterfaceGL::GetAlpha(CCEffectType type)
{
    int nPart = (int)m_AllEffectPart.size();
    float Alpha =0.0;
    for (int i=0;i<nPart;i++) {
        if(m_BasePart[i]->m_EffectPart == type)
        {
            Alpha = m_BasePart[i]->m_alpha;
            break;
        }

    }
	if (type == LUT_ADJUST_EFFECT || type == LUT_CLEAR_EFFECT || TYPE_FILTER_EFFECT)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlterGL *part = (CLUTFIlterGL*)m_BasePart[i];
				if (type == LUT_ADJUST_EFFECT)
				{
					Alpha = part->m_alphaAdjust;
				}
				else if (type == LUT_CLEAR_EFFECT)
				{
					Alpha = part->m_alphaClear;
				}
				else if (type == TYPE_FILTER_EFFECT)
				{
					Alpha = part->m_alphaType;
				}
				break;
			}
		}
	}

	//眼影 双眼皮 眉毛 嘴巴
	if (type == FACE_EYESTRICK || type == FACE_EYEBROWSTRICK || type == FACE_MOUTH_STRICK || type == FACE_EYELID)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{

					if ((part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE && type == FACE_EYESTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE_LID && type == FACE_EYELID) || 
						(part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYEBROW && type == FACE_EYEBROWSTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_MOUTH && type == FACE_MOUTH_STRICK))
					{
						Alpha = part->m_vMakeUpBlendInfo[j].alpha;
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
			if (m_BasePart[i]->m_EffectPart == FACE_LIPSTRICK)
			{
				CFaceLipstickGL *part = (CFaceLipstickGL*)m_BasePart[i];
				Alpha = part->m_wihteTeethAlpha;
				break;
			}
		}
	}
	else if (type == FACE_REMOVE_POUCH || type == FACE_BRIGHT_EYE)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_REMOVE_FALIN)
			{
				CFaceRemovePouchFalinGL* part = (CFaceRemovePouchFalinGL*)m_BasePart[i];
				if (type == FACE_REMOVE_POUCH)
				{
					Alpha = part->m_pouchAlpha ;
				}
				else
				{
					Alpha = part->m_BrightEyeAlpha;
				}

				break;
			}
		}
	}

    return Alpha;
}

void CCEffectInterfaceGL::facePointIndexTransfer(const ccFDFaceRes* faceRes, std::vector<float>& mDstPoint74)
{
   
    Vector2 *pSrcPoint130 = (Vector2*)faceRes->arrFace[0].arrShapeDense;
    Vector2 nDstPoint74[74];
    for (int i = 0; i < 9; i++)
    {
        nDstPoint74[i] = pSrcPoint130[i * 2];
    }
    for (int j = 9; j < 17; j++)
    {
        int index = 50 - 2 * j;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    for (int j = 17; j < 22; j++)
    {
        int index = j + 16;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    for (int j = 22; j < 27; j++)
    {
        int index = j + 20;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    for (int j = 22; j < 27; j++)
    {
        int index = j + 20;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    for (int j = 27; j < 31; j++)
    {
        int index = j + 44;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    for (int j = 31; j < 36; j++)
    {
        int index = j + 47;
        if (j == 32 || j == 34)
        {
            nDstPoint74[j] = (pSrcPoint130[index - 1] + pSrcPoint130[index + 1]) *0.5;
        }
        else
        {
            nDstPoint74[j] = pSrcPoint130[index];
        }
    }
    for (int j = 36; j < 45; j++)
    {
        int index = j + 15;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    for (int j = 45; j < 54; j++)
    {
        int index = j + 16;
        nDstPoint74[j] = pSrcPoint130[index];
    }
    nDstPoint74[54] = pSrcPoint130[86];
    nDstPoint74[55] = pSrcPoint130[88];
    nDstPoint74[56] = pSrcPoint130[87];
    nDstPoint74[57] = pSrcPoint130[98];
    nDstPoint74[58] = pSrcPoint130[99];

    nDstPoint74[59] = pSrcPoint130[92];
    nDstPoint74[60] = pSrcPoint130[90];
    nDstPoint74[61] = pSrcPoint130[91];
    nDstPoint74[62] = pSrcPoint130[102];
    nDstPoint74[63] = pSrcPoint130[101];

    nDstPoint74[64] = pSrcPoint130[89];
    nDstPoint74[65] = pSrcPoint130[100];

    nDstPoint74[66] = (pSrcPoint130[103] + pSrcPoint130[109])*0.5;
    nDstPoint74[66].y = pSrcPoint130[109].y;
    nDstPoint74[67] = pSrcPoint130[93];
    nDstPoint74[68] = pSrcPoint130[94];

    nDstPoint74[69] = (pSrcPoint130[105] + pSrcPoint130[108])*0.5;
    nDstPoint74[69].y = pSrcPoint130[108].y;
    nDstPoint74[70] = pSrcPoint130[97];
    nDstPoint74[71] = pSrcPoint130[96];

    nDstPoint74[72] = pSrcPoint130[104];
    nDstPoint74[73] = pSrcPoint130[95];

    std::vector<float> nPoint(148);
    for (int k = 0; k < 74; k++)
    {
        nPoint[k] = nDstPoint74[k].x;
        nPoint[k + 74] = nDstPoint74[k].y;
    }
    mDstPoint74 = nPoint;
}

void CCEffectInterfaceGL::RealeseEffectPart()
{
	if (m_effectStatus == CES_LOADING)
	{
		m_ReleaseStatus = true;
	}
	for (int i = 0; i < m_AllEffectPart.size(); ++i)
	{
		SAFE_DELETE(m_AllEffectPart[i]);
	}
	m_AllEffectPart.clear();

}

void CCEffectInterfaceGL::RealeseBasePart()
{
    for (int i = 0; i < m_BasePart.size(); ++i)
    {
        SAFE_DELETE(m_BasePart[i]);
    }
    m_BasePart.clear();
}



void *CCEffectInterfaceGL::ReadConfigThread(void *pM)
{
	CCResourceAsyn *pResourceAsyn = (CCResourceAsyn *)pM;
	const char *szPath = pResourceAsyn->szPath.c_str();
	const char *szXMLFile = pResourceAsyn->szXMLFile.c_str();
	HZIP hZip = OpenZip(szPath, NULL);
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

            if(nodeEffect.isEmpty())
            {
                CEffectPart *pPart = NULL;
                pPart = new CStickerEffectGL();
                pPart->SetResourcePath(pResourceAsyn->m_resourcePath);
                pPart->ReadConfig(nodeModels, hZip, (char*)pResourceAsyn->szPath.c_str());
                //pPart->Prepare();
                pResourceAsyn->m_UseDepth = true;
                pResourceAsyn->m_AllEffectPart.push_back(pPart);
            }
			while (!nodeEffect.isEmpty())
			{
				const char *szType = nodeEffect.getAttribute("type");
				CEffectPart *pPart = NULL;
				if (szType != NULL && !strcmp(szType, "FaceEffect2D"))
				{
                    pPart = new CFaceEffect2DGL();
				}
                else if (szType != NULL && !strcmp(szType, "FaceEffect2DWithBG"))
                {
                    pPart = new CFaceEffect2DBlendBGGL();
                }
				else if (szType != NULL && !strcmp(szType, "FaceBlush"))
				{
					pPart = new CFaceEffect2DBlendBGGL();
					CFaceEffect2DBlendBGGL* p = (CFaceEffect2DBlendBGGL*)pPart;
					p->m_EffectPart = FACE_BLUSH;
					p->m_alpha = 0.0;
				}
				else if (szType != NULL && (!strcmp(szType, "MakeUpBlend") || !strcmp(szType, "MakeUp")))
				{
					pPart = new CFaceEffectBlendMakeUp();
					pPart->m_EffectPart = FACE_EYESTRICK;
				}
                else if (szType != NULL && !strcmp(szType, "FaceMeshChange"))
                {
                    pPart = new CFaceMeshChangeGL();
                }
                else if (szType != NULL && !strcmp(szType, "FaceEffect3DModel"))
                {
                    pPart = new CFaceEffect3DModelGL();
                    pResourceAsyn->m_UseDepth = true;
                }
                else if (szType != NULL && !strcmp(szType, "FaceMLS"))
                {
                    pPart = new CFaceSPMLSGL();
                }
                else if (szType != NULL && !strcmp(szType, "InsSmooth"))
                {
                    pPart = new CFaceSmoothInsGL();
                }
				else if (szType != NULL && !strcmp(szType, "FaceLevel"))
				{
					pPart = new CFaceLevelGL();
					CFaceLevelGL* p = (CFaceLevelGL*)pPart;
					p->m_EffectPart = FACE_LEVELS;
					p->m_alpha = 0.0;
				}
                else if (szType != NULL && !strcmp(szType, "LUTFilter"))
                {
                    pPart = new CLUTFIlterGL();
                }
                else if (szType != NULL && !strcmp(szType, "FoodieSharp"))
                {
                    pPart = new CFoodieSharpGL();
                }
                else if (szType != NULL && !strcmp(szType, "FaceHighlight"))
                {
                    pPart = new CFaceShadowHighLightGL();
                }
                else if (szType != NULL && !strcmp(szType, "Denoise"))
                {
                    pPart = new CDeNosieFilterGL();
                }
				else if (szType != NULL && !strcmp(szType, "FaceShapeBeauty"))
				{
					pPart = new CFaceBeautyGL();
				}
				else if (szType != NULL && !strcmp(szType, "FaceLipstick"))
				{
					pPart = new CFaceLipstickGL();
				}

				else if (szType != NULL && !strcmp(szType, "FaceSoftLight"))
				{
					//脸部柔光
					pPart = new CFaceEffect2DBlendBGGL();
					CFaceEffect2DBlendBGGL* p = (CFaceEffect2DBlendBGGL*)pPart;
					p->m_EffectPart = FACE_BLEND_BLUSH;
					p->m_alpha = 0.0;

				}
				else if (szType != NULL && !strcmp(szType, "RemovePouchFalin"))
				{
					pPart = new CFaceRemovePouchFalinGL();
				}
				else if (szType != NULL && !strcmp(szType, "FaceEffect2DSticker"))
				{
					pPart = new CFaceEffect2DStickerGL();
				}
				//else if (szType != nullptr && !strcmp(szType, "WallPaperPBRModel"))
				//{
				//	pPart = new CWallPaperPBRModel();
				//}
				else if (szType != NULL && !strcmp(szType, "FacePBRModel"))
				{
					pPart = new CFacePBRModelGL();
					pResourceAsyn->m_UseDepth = true;
				}
				else
				{
					pPart = new CEffectPart();
				}
				pPart->SetResourcePath(pResourceAsyn->m_resourcePath);
				pPart->ReadConfig(nodeEffect, hZip, (char*)pResourceAsyn->szPath.c_str());
                //pPart->Prepare();
				pResourceAsyn->m_AllEffectPart.push_back(pPart);
				nodeEffect = nodeModels.getChildNode("typeeffect", ++i);
			}
		}
	}
	CloseZip(hZip);
	pResourceAsyn->status = CES_READY;
	if (m_pEffectCallback != NULL)
	{
		(*m_pEffectCallback)(EE_READY, m_lpParam);
	}
    return NULL;

}

void CCEffectInterfaceGL::renderEffect(GLuint pInputTexture, int width, int height, const ccFDFaceRes * faceRes)
{

}

void CCEffectInterfaceGL::loadEffectFromZip_asyn_impl(const char * szPath, const char *szXMLFile)
{
	
	m_resourceAsyn.reset();
    m_resourceAsyn.m_AllEffectPart.clear();
    m_resourceAsyn.szPath = szPath;
    m_resourceAsyn.szXMLFile = szXMLFile;
    m_resourceAsyn.m_resourcePath = m_resourcePath;
    std::thread th(&CCEffectInterfaceGL::ReadConfigThread, this, &m_resourceAsyn);
    th.detach();
    m_effectStatus = CES_LOADING;
}

void CCEffectInterfaceGL::loadEffectFromZip_sync_impl(const char * szPath, const char *szXMLFile)
{
    m_resourceAsyn.reset();
	m_resourceAsyn.m_AllEffectPart.clear();
	m_resourceAsyn.szPath = szPath;
	m_resourceAsyn.szXMLFile = szXMLFile;
	m_resourceAsyn.m_resourcePath = m_resourcePath;
	ReadConfigThread(&m_resourceAsyn);
}

void CCEffectInterfaceGL::touchThreadSource()
{
	if (m_resourceAsyn.status == CES_READY)
	{
        //CloseHandle(m_hLoadThread);
        //m_hLoadThread = 0;
		m_effectStatus = CES_IDLE;
		RealeseEffectPart();
		m_AllEffectPart = m_resourceAsyn.m_AllEffectPart;
		
        m_useDepth = m_resourceAsyn.m_UseDepth;
		m_resourceAsyn.reset();
		m_nStartTime = -1;
		if (m_LastZip != "")
		{
			std::string strLastZip(m_LastZip);
			RealeseEffectPart();
			loadEffectFromZipAsyn(strLastZip, "test.xml");
			m_LastZip = "";
		}
		if (m_ReleaseStatus)
		{
			RealeseEffectPart();
			m_ReleaseStatus = false;
		}
        for (int i=0;i<m_AllEffectPart.size();i++) {
            m_AllEffectPart[i]->Prepare();
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
}

bool CCEffectInterfaceGL::loadEffectFromZipAsyn(const std::string &szZipFile, const std::string &xml, EffectCallback callback, void * lpParam)
{
	m_ReleaseStatus = false;
    touchThreadSource();
	if (m_effectStatus == CES_LOADING)
	{
		std::string strZipPath(szZipFile);
		m_LastZip = strZipPath;
		return false;
	}
	if (szZipFile.size() > 0)
	{
		m_pEffectCallback = callback;
		m_lpParam = lpParam;
        loadEffectFromZip_asyn_impl(szZipFile.c_str(),xml.c_str());
		touchThreadSource();
	}
	return true;
}

bool CCEffectInterfaceGL::loadEffectFromZipSync(const std::string & szZipFile, const std::string & xml, EffectCallback callback, void * lpParam)
{
	m_ReleaseStatus = false;
	if (m_effectStatus == CES_LOADING)
	{
		return false;
	}
	if (szZipFile.size() > 0)
	{
		m_pEffectCallback = callback;
		m_lpParam = lpParam;
		loadEffectFromZip_sync_impl(szZipFile.c_str(), xml.c_str());
        touchThreadSource();
	}

	return true;
}

bool CCEffectInterfaceGL::loadBaseFromZipSync(const std::string &szZipFile, const std::string &xml)
{
	m_ReleaseStatus = false;
    if (m_effectStatus == CES_LOADING)
    {
        return false;
    }
    if (szZipFile.size() > 0 && szZipFile.size() < 512)
    {
        m_resourceAsyn.reset();
        m_resourceAsyn.m_AllEffectPart.clear();
        m_resourceAsyn.szPath = szZipFile;
        m_resourceAsyn.szXMLFile = xml;
        m_resourceAsyn.m_resourcePath = m_resourcePath;
        ReadConfigThread(&m_resourceAsyn);
		m_useDepth = m_resourceAsyn.m_UseDepth;
		m_nStartTime = -1;
        RealeseBasePart();
        m_BasePart = m_resourceAsyn.m_AllEffectPart;
        m_resourceAsyn.reset();
        for (int i=0;i<m_BasePart.size();i++) {
            m_BasePart[i]->Prepare();
        }

    }

    return true;
}


bool CCEffectInterfaceGL::renderEffectToTexture(GLuint pInputTexture, GLuint pTargetTexture, int width, int height, const ccFDFaceRes * faceRes, FaceOriImageRect *pFaceRect)
{
	touchThreadSource();
//	if (m_effectStatus != CES_RUN)
//	{
//		return false;
//	}

    if (m_AllEffectPart.size() == 0 && m_BasePart.size() ==0)
	{
		return false;
	}


    if (m_nStartTime < 0)
    {
        m_nStartTime = ccCurrentTime();
    }

    long long  currentTime = ccCurrentTime();
    long runTime = long(currentTime - m_nStartTime);

	if (m_effectStatus != CES_LOADING && m_resourceAsyn.m_nEffectDuring > 0 && runTime >= m_resourceAsyn.m_nEffectDuring)
    {
        m_effectStatus = CES_IDLE;
        if (m_pEffectCallback != NULL)
        {
            (*m_pEffectCallback)(EE_FINISHED, m_lpParam);
        }
        //return false;
    }
    if (m_DoubleBuffer == NULL || m_DoubleBuffer->GetWidth() != width || m_DoubleBuffer->GetHeight() != height)
    {
       
		m_DoubleBuffer = std::make_shared<GLDoubleBuffer>();
        m_DoubleBuffer->InitDoubleBuffer(pTargetTexture,width, height,true);
		if (m_renderParam!=NULL)
		{
			delete m_renderParam;
		}
        m_renderParam = new BaseRenderParam();
        m_renderParam->SetDoubleBuffer(m_DoubleBuffer);

    }
    m_DoubleBuffer->UpdtaeTexA(pTargetTexture);
	m_DoubleBuffer->BindFBOA();
    m_DoubleBuffer->FilterToTex(pInputTexture);

	std::shared_ptr<CC3DTextureRHI> SrcTex = GetDynamicRHI()->CreateTexture();
	RHIResourceCast(SrcTex.get())->AttatchTextureId(pInputTexture);
    m_renderParam->SetSrcTex(SrcTex);
    if(faceRes == NULL)
    {
        m_renderParam->SetFaceNum(1);
        float Point[] = {
            241.026,504.62,241.449,534.423,244.127,564.092,248.722,593.753,254.507,622.859,261.471,651.367,268.724,679.965,277.934,707.969,290.455,734.695,305.903,759.71,323.843,782.998,343.466,804.891,364.103,825.673,385.733,845.133,408.837,862.235,435.915,873.914,466.089,877.792,495.7,873.551,522.983,862.05,545.748,844.354,566.536,824.179,586.184,802.791,604.816,780.264,621.694,756.178,636.275,730.556,647.931,703.207,656.514,674.497,663.149,645.571,668.798,616.381,673.744,586.893,677.084,557.164,678.441,527.442,677.867,497.392,276.669,465.979,301.685,435.765,337.109,427.136,373.493,428.348,407.483,436.656,404.002,459.094,371.923,454.256,338.544,452.532,306.128,456.876,506.929,435.983,540.684,426.282,576.593,423.657,612.263,430.979,638.48,459.235,608.846,451.913,576.146,448.835,543.155,452.175,511.328,458.233,317.206,518.152,338.149,504.621,363.591,501.178,388.244,508.815,405.417,527.279,382.629,531.103,358.853,533.576,336.289,528.838,360.353,515.656,361.075,516.826,510.753,526.403,526.955,506.483,551.557,497.95,577.387,500.656,598.508,513.606,580.137,525.261,557.688,531.124,533.823,529.634,553.519,513.124,555.119,514.155,458.362,507.397,459.345,552.474,460.216,597.396,461.124,641.573,428.059,519.667,419.326,616.923,401.876,646.165,415.149,664.359,431.152,662.504,461.946,677.077,491.79,661.572,507.706,663.836,520.471,644.934,502.082,615.986,488.985,518.777,376.465,717.273,407.623,712.553,441.519,711.157,463.457,715.132,485.166,710.746,518.645,711.825,549.217,716.299,530.201,746.192,502.502,769.536,463.571,778.158,424.518,769.887,396.355,746.95,386.882,720.331,411.209,721.871,463.445,728.311,515.337,720.979,538.999,719.212,517.784,734.945,462.986,750.369,408.019,735.971,440.231,725.471,486.719,725.238,434.207,746.1,491.857,745.66,391.443,713.83,424.324,711.323,502.274,711.101,534.653,712.97,340.951,516.253,379.857,515.055,533.841,513.139,573.301,512.661,
        };

        m_renderParam->SetFacePoint130((Vector2*)Point, 0, width, height);

		FacePosInfo pFaceInfo;
		pFaceInfo.faceW = 500;
		pFaceInfo.faceH = 500;

        float mat[]={0.999807,-0.018005,0.007917,0.000000,-0.019495,-0.960491,0.277626,0.000000,-0.002606,0.277727,0.960656,0.000000,-1.323400,-20.464117,-321.352539,1.000000};
        memcpy(pFaceInfo.arrWMatrix, mat, 16 * sizeof(float));
        pFaceInfo.pitch = -16;
        pFaceInfo.roll = 1.11;
        pFaceInfo.yaw = 0.149;
        pFaceInfo.x = -1.323;
        pFaceInfo.y = 20.46;
        pFaceInfo.z = 321.35;

		pFaceInfo.pCameraRect.x = width/4;
		pFaceInfo.pCameraRect.y = height/4;
		pFaceInfo.pCameraRect.width = width/2;
		pFaceInfo.pCameraRect.height = height/2;
        m_renderParam->SetFacePosInfo(&pFaceInfo, 0);
    }

    else
    {
        m_renderParam->SetFaceNum(faceRes->numFace);
        for (int n = 0; n < faceRes->numFace; n++)
        {
			Vector2 *pSrcFacePoint = (Vector2*)faceRes->arrFace[n].arrShapeDense;
			if (pFaceRect != NULL &&pFaceRect->width >0)
			{
				Vector2 FacePoint[130];
				
				for (int i=0;i<130;i++)
				{
					FacePoint[i] = pSrcFacePoint[i] * pFaceRect->Scalexy + Vector2(pFaceRect->x, pFaceRect->y);
				}
				m_renderParam->SetFacePoint130(FacePoint, n, width, height);
			}
			else
			{
				m_renderParam->SetFacePoint130(pSrcFacePoint, n, width, height);
			}
            
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
			if (pFaceRect)
			{
				pFaceInfo.pFaceRect = *pFaceRect;
			}
			
			
			pFaceInfo.pCameraRect.x = faceRes->arrFace[n].gesture_box_left;
			pFaceInfo.pCameraRect.y = faceRes->arrFace[n].gesture_box_top;
			pFaceInfo.pCameraRect.width = faceRes->arrFace[n].gesture_box_right - faceRes->arrFace[n].gesture_box_left;
			pFaceInfo.pCameraRect.height = faceRes->arrFace[n].gesture_box_bottom - faceRes->arrFace[n].gesture_box_top;

            m_renderParam->SetFacePosInfo(&pFaceInfo, n);
        }
    }
	m_renderParam->SetExpressionCoffes(m_Coffes);
    m_renderParam->runtime = runTime;
    for (int i = 0; i < m_BasePart.size(); i++)
    {
        m_BasePart[i]->Render(*m_renderParam);
    }

    if(m_effectStatus == CES_RUN)
    {
        for (int i = 0; i < m_AllEffectPart.size(); i++)
        {
            m_AllEffectPart[i]->Render(*m_renderParam);
        }
    }
// 	cv::Mat mImag = cv::Mat(height, width, CV_8UC4);
// 	m_DoubleBuffer->m_pFBOA->ReadPixels(mImag.data);
	if (pTargetTexture != m_DoubleBuffer->GetFBOTextureAID())
	{
		m_DoubleBuffer->SyncAToB();
		m_DoubleBuffer->SwapFBO();
	}
	m_DoubleBuffer->unBindFBOA();
    return true;
}

void CCEffectInterfaceGL::SetBGRA(unsigned char * pRGBA)
{
	if (m_renderParam != NULL)
	{
		m_renderParam->m_pBGRA_Src = pRGBA;
	}
}

void CCEffectInterfaceGL::SetMatrial(unsigned char * pMaterial, int nWidth, int nHeight, CCEffectType type)
{
	if (type == LUT_FILTER_EFFECT)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++)
		{
			if (m_BasePart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlterGL *part = (CLUTFIlterGL*)(m_BasePart[i]);
				part->m_material->UpdateTextureInfo(pMaterial, nWidth, nHeight);
				//part->m_material.bind(0, pMaterial, nWidth, nHeight);
				break;
			}
		}
	}
	else if (type == LUT_ADJUST_EFFECT)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++)
		{
			if (m_BasePart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlterGL*part = (CLUTFIlterGL*)(m_BasePart[i]);
				part->m_materialAdjust->UpdateTextureInfo(pMaterial, nWidth, nHeight);
				//part->m_materialAdjust.bind(0, pMaterial, nWidth, nHeight);
				break;
			}
		}
	}
	else if (type == LUT_CLEAR_EFFECT)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++)
		{
			if (m_BasePart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlterGL*part = (CLUTFIlterGL*)(m_BasePart[i]);
				part->m_materialClear->UpdateTextureInfo(pMaterial, nWidth, nHeight);
				break;
			}
		}
	}
	else if (type == TYPE_FILTER_EFFECT)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++)
		{
			if (m_BasePart[i]->m_EffectPart == LUT_FILTER_EFFECT)
			{
				CLUTFIlterGL *part = (CLUTFIlterGL*)(m_BasePart[i]);
				part->m_materialType->UpdateTextureInfo(pMaterial, nWidth, nHeight);
				break;
			}
		}
	}
	else if (type == FACE_EYESTRICK)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp  *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{
					if (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE)
					{
						//MaterialTex *pTex = part->m_vMakeUpBlendInfo[j].m_drawable->getTex(0);
						//pTex->bind(0, pMaterial, nWidth, nHeight);
						auto TexRHI = part->m_vMakeUpBlendInfo[j].m_drawable->GetTex(0);
						TexRHI->UpdateTextureInfo(pMaterial, nWidth, nHeight);
						break;
					}
				}
				break;
			}
		}
	}
	else if (type == FACE_EYEBROWSTRICK)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp  *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{
					if (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYEBROW)
					{
						//MaterialTex *pTex = part->m_vMakeUpBlendInfo[j].m_drawable->getTex(0);
						//pTex->bind(0, pMaterial, nWidth, nHeight);
						auto TexRHI = part->m_vMakeUpBlendInfo[j].m_drawable->GetTex(0);
						TexRHI->UpdateTextureInfo(pMaterial, nWidth, nHeight);
						break;
					}
				}
				break;
			}
		}
	}

	else if (type == FACE_MOUTH_STRICK)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp  *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{
					if (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_MOUTH)
					{
						//MaterialTex *pTex = part->m_vMakeUpBlendInfo[j].m_drawable->getTex(0);
						//pTex->bind(0, pMaterial, nWidth, nHeight);
						auto TexRHI = part->m_vMakeUpBlendInfo[j].m_drawable->GetTex(0);
						TexRHI->UpdateTextureInfo(pMaterial, nWidth, nHeight);
						break;
					}
				}
				break;
			}
		}
	}

	else if (type == FACE_EYELID)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp  *)m_BasePart[i];
				part->m_IsEyelidVisable = true;
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{
					if (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE_LID)
					{
						//MaterialTex *pTex = part->m_vMakeUpBlendInfo[j].m_drawable->getTex(0);
						//pTex->bind(0, pMaterial, nWidth, nHeight);
						auto TexRHI = part->m_vMakeUpBlendInfo[j].m_drawable->GetTex(0);
						TexRHI->UpdateTextureInfo(pMaterial, nWidth, nHeight);
						break;
					}
				}
				break;
			}
		}
	}

	else if (type == FACE_BLUSH)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++)
		{
			if (m_BasePart[i]->m_EffectPart == FACE_BLUSH)
			{
				CFaceEffect2DBlendBGGL *part = (CFaceEffect2DBlendBGGL*)m_BasePart[i];

				//MaterialTex *pTex = part->m_Drawable->getTex(0);
				//pTex->bind(0, pMaterial, nWidth, nHeight);
				auto TexRHI = part->m_Drawable->GetTex(0);
				TexRHI->UpdateTextureInfo(pMaterial, nWidth, nHeight);
				break;
			}
		}
	}

}

void CCEffectInterfaceGL::SetMatrial(const char * fileName, CCEffectType type)
{

	if (fileName !=NULL)
	{
		int nWidth, nHeight,n;
		BYTE *pImage = ccLoadImage(fileName, &nWidth, &nHeight, &n, 4);
		if (pImage != NULL && nWidth > 0)
		{
			SetMatrial(pImage, nWidth, nHeight, type);
			SAFE_DELETE_ARRAY(pImage);
		}
		else
		{
			#ifdef PLATFORM_ANDROID
			LOGD("[ccfilter] fileName:-------------->%s", fileName);
			#endif
		}
	}
	else if(type == FACE_EYELID)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp  *)m_BasePart[i];
				part->m_IsEyelidVisable = false;
				break;
			}
		}
	}
}

void CCEffectInterfaceGL::SetMakeUpZip(const std::string & szZipFile)
{
	HZIP hZip = OpenZip(szZipFile.c_str(), NULL);

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
				return;
			}
			XMLResults xResults;
			XMLNode nodeModels = XMLNode::parseBuffer(pDataBuffer, ze.unc_size, "models", &xResults);
			int ii = -1;
			XMLNode nodeEffect = nodeModels.getChildNode("typeeffect", ++ii);
			bool HasEye = false;
			bool HasMouth = false;
			while (!nodeEffect.isEmpty())
			{
				const char *szType = nodeEffect.getAttribute("type");

				if (szType != NULL && (!strcmp(szType, "MakeUpBlend")))
				{
					int nPart = (int)m_BasePart.size();
					CFaceEffectBlendMakeUp *part = NULL;
					for (int i = 0; i < nPart; i++) {
						if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
						{
							part = (CFaceEffectBlendMakeUp  *)m_BasePart[i];
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
							if (!HasMouth)
							{
								SetColor(0, 0, 0, 0);
							}
			
						}

						float alpha = 0.0f;
						const char *szAlpha = nodeDrawable.getAttribute("alpha");
						if (szAlpha != NULL)
						{
							alpha = atof(szAlpha);
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
						int w = 0, h =0 ;
						BYTE *pData = CreateImgFromZIP(hZip, szImagePath, w, h);

						for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
						{
							if (part->m_vMakeUpBlendInfo[j].m_meshType == m_meshType)
							{
								part->m_vMakeUpBlendInfo[j].m_blendType = m_blendType;
								//MaterialTex *pTex = part->m_vMakeUpBlendInfo[j].m_drawable->getTex(0);
								std::shared_ptr<MaterialTexRHI> TexRHI = part->m_vMakeUpBlendInfo[j].m_drawable->GetTex(0);
								part->m_vMakeUpBlendInfo[j].alpha = alpha;
								if (w > 0 && h > 0)
								{
									//pTex->bind(0, pData, w, h);
									TexRHI->UpdateTextureInfo(pData, w, h);
								}
								break;
							}

						}
						SAFE_DELETE_ARRAY(pData);
					}
				}

				else if (szType != NULL && (!strcmp(szType, "FaceBlush")))
				{
					int nPart = (int)m_BasePart.size();
					CFaceEffect2DBlendBGGL *part = NULL;
					for (int i = 0; i < nPart; i++) {
						if (m_BasePart[i]->m_EffectPart == FACE_BLUSH)
						{
							part = (CFaceEffect2DBlendBGGL*)m_BasePart[i];
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

						float alpha = 0.0f;
						const char *szAlpha = nodeDrawable.getAttribute("alpha");
						if (szAlpha != NULL)
						{
							alpha = atof(szAlpha);
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
						int w = 0, h = 0;
						BYTE *pData = CreateImgFromZIP(hZip, szImagePath, w, h);
						part->m_BlendType = m_blendType;
						part->m_alpha = alpha;
						//MaterialTex *pTex = part->m_Drawable->getTex(0);
						std::shared_ptr<MaterialTexRHI> TexRHI = part->m_Drawable->GetTex(0);
						if (w>0&&h>0)
						{
							TexRHI->UpdateTextureInfo(pData, w, h);
							//pTex->bind(0, pData, w, h);
						}
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
						float alpha = 0.0f;
						const char *szAlpha = node.getAttribute("alpha");
						if (szAlpha != NULL)
						{
							alpha = atof(szAlpha);
						}
						setAlpha(alpha, FACE_LIPSTRICK);
						HasMouth = true;
					}
				}


				nodeEffect = nodeModels.getChildNode("typeeffect", ++ii);
			}

		}
	}
	CloseZip(hZip);
}

void CCEffectInterfaceGL::SetExpression(std::vector<float>& coeffs)
{
	m_Coffes = coeffs;
}


void CCEffectInterfaceGL::SetColor(float r, float g, float b, float a, CCEffectType type)
{
	if (type == FACE_LIPSTRICK)
	{
		int nPart = (int)m_BasePart.size();
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_LIPSTRICK)
			{
				CFaceLipstickGL *part = (CFaceLipstickGL*)m_BasePart[i];
				part->m_MouthRGBA = Vector4(r, g, b, a) / 255.f;
				break;
			}
		}
	}
}

void CCEffectInterfaceGL::setBlend(BlendType blendType, CCEffectType type)
{
	int nPart = (int)m_BasePart.size();

	if (type == FACE_BLUSH)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == type)
			{
				CFaceEffect2DBlendBGGL *part = (CFaceEffect2DBlendBGGL*)m_BasePart[i];
				part->m_BlendType = blendType;
				break;
			}
		}
	}

	//眼影 双眼皮 眉毛 嘴巴的混合方式
	if (type == FACE_EYESTRICK || type == FACE_EYEBROWSTRICK || type == FACE_MOUTH_STRICK || type == FACE_EYELID)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{
					if ((part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE && type == FACE_EYESTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE_LID && type == FACE_EYELID) || 
						(part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYEBROW && type == FACE_EYEBROWSTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_MOUTH && type == FACE_MOUTH_STRICK))
					{

						part->m_vMakeUpBlendInfo[j].m_blendType = blendType;
						break;
					}
				}
				break;
			}
		}
	}

}

void CCEffectInterfaceGL::setBlendType(int index, CCEffectType type)
{
	int nPart = (int)m_BasePart.size();

	if (type == FACE_BLUSH)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == type)
			{
				CFaceEffect2DBlendBGGL *part = (CFaceEffect2DBlendBGGL*)m_BasePart[i];

				if (index == 0)
				{
					part->m_BlendType = NORMAL_TYPE;
				}
				else if (index == 1)
				{
					part->m_BlendType = MULTIPLY_TYPE;
				}
				else
				{
					part->m_BlendType = SOFTLIGHT_TYPE;
				}
				break;


			}
		}
	}


	if (type == FACE_EYESTRICK || type == FACE_EYEBROWSTRICK || type == FACE_MOUTH_STRICK || type == FACE_EYELID)
	{
		for (int i = 0; i < nPart; i++) {
			if (m_BasePart[i]->m_EffectPart == FACE_EYESTRICK)
			{
				CFaceEffectBlendMakeUp *part = (CFaceEffectBlendMakeUp *)m_BasePart[i];
				for (int j = 0; j < part->m_vMakeUpBlendInfo.size(); j++)
				{
					if ((part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE && type == FACE_EYESTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYE_LID && type == FACE_EYELID) || 
						(part->m_vMakeUpBlendInfo[j].m_meshType == MESH_EYEBROW && type == FACE_EYEBROWSTRICK) || (part->m_vMakeUpBlendInfo[j].m_meshType == MESH_MOUTH && type == FACE_MOUTH_STRICK))
					{
						if (index == 0)
						{
							part->m_vMakeUpBlendInfo[j].m_blendType = NORMAL_TYPE;
						}
						else if (index == 1)
						{
							part->m_vMakeUpBlendInfo[j].m_blendType = MULTIPLY_TYPE;
						}
						else
						{
							part->m_vMakeUpBlendInfo[j].m_blendType = SOFTLIGHT_TYPE;
						}
						break;
					}
				}
				break;
			}
		}
	}

}

void CCEffectInterfaceGL::SetResourcePath(char *path)
{
	m_resourcePath = path;
	CC3DEnvironmentConfig::getInstance()->resourth_path = m_resourcePath;
}

void CCEffectInterfaceGL::SetIOSDevice(bool isIpad, CCEffectType type)
{
	//int nPart = (int)m_AllEffectPart.size();
	//for (int i = 0; i < nPart; i++) {
	//	if (m_AllEffectPart[i]->m_EffectPart == PBR_3D_MODEL && type == PBR_3D_MODEL)
	//	{
	//		CPBRModel *part = (CPBRModel*)(m_AllEffectPart[i]);
	//		if (part->m_Render != NULL && !part->m_3DScene->m_hasLoadModel)
	//		{
	//			part->m_Render->m_MsaaFrameBuffer->m_isIpad = isIpad;
	//		}
	//		break;
	//	}
	//}

}
