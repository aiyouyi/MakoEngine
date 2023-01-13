#include "CBodyBGEffect.h"
#include "common.h"
#include <xnamath.h>
#include<BaseDefine/Vectors.h>
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

struct RectConstantMask
{
	XMMATRIX mWVP; //��Ͼ���
	XMFLOAT4 mClip;//�����ü�����
};

CBodyBGEffect::CBodyBGEffect()
{
	m_EffectPart = SEG_EFFECT_BACKGROUND;
	m_nEffectDuring = -1;
	m_nAniLoopTime = -1;
	m_nStartTime = -1;
	m_nLoopStartTime = -1;

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBufferMask = NULL;
}


CBodyBGEffect::~CBodyBGEffect()
{
	
	Release();
}

void * CBodyBGEffect::Clone()
{
	CBodyBGEffect* result = new CBodyBGEffect();
	*result = *this;
	return result;
}

bool CBodyBGEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CBodyBGEffect::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CBodyBGEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* szDrawableName = nodeDrawable.getAttribute("name");

			//get items info
			const char *szItems = nodeDrawable.getAttribute("items");
			const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
			if (szItems != NULL && szItemInfo != NULL)
			{
				int iStart = 0;
				int nCount = 0;
				int nDuring = 0;
				int nStep = 1;
				char szImagePath[128];
				sscanf(szItemInfo, "%d,%d,%d,%d", &iStart, &nCount, &nDuring, &nStep);
				if (nCount >= 0 && nDuring > 0)
				{
					char szFullFile[256];
					AnimInfo anim_info;
					anim_info.fps = 1000.0 / float(nDuring);
					anim_info.dir = path;

					for (; iStart <= nCount; iStart += nStep)
					{
						sprintf(szImagePath, szItems, iStart);

						if (hZip == 0)
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
							anim_info.relative_filename_list.push_back(szImagePath);
						}
						else
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
							anim_info.relative_filename_list.push_back(szImagePath);
						}

					}

					if (hZip == 0)
					{
						m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
					}
					else
					{
						m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
					}
				}
			}

		}

		XMLNode nodeTime = childNode.getChildNode("time", 0);
		if (!nodeTime.isEmpty())
		{
			const char* szDelay = nodeTime.getAttribute("delay");
			const char* szPlay = nodeTime.getAttribute("play");
			const char* szAll = nodeTime.getAttribute("all");
			if (szDelay != NULL)
			{
				m_play.delaytime = atoi(szDelay);
			}
			if (szPlay != NULL)
			{
				m_play.playtime = atoi(szPlay);
			}

			if (szAll != NULL)
			{
				m_play.alltime = atoi(szAll);
			}
		}
	}
	return false;
}

bool CBodyBGEffect::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	string path = m_resourcePath + "/Shader/rectDrawWithMask.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);


	float vertices[] =
	{
		-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,0.0f, 1.0f,
		1.0f, -1.0f, 0.0f,1.0f, 1.0f,
		1, 1.0f, 0.0f ,1.0f, 0.0f
	};

	m_rectVerticeBuffer = DXUtils::CreateVertexBuffer(vertices, 4, 5);

	unsigned short index[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	m_rectIndexBuffer = DXUtils::CreateIndexBuffer(index, 2);

	m_pConstantBufferMask = DXUtils::CreateConstantBuffer(sizeof(RectConstantMask));
	return true;
}

void CBodyBGEffect::Render(BaseRenderParam & RenderParam)
{
	auto BodyTexture = RenderParam.GetBodyMaskTexture();
	if (BodyTexture == NULL)
	{
		return;
	}

	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();


	if (m_nStartTime < 0)
	{
		m_nLoopStartTime = m_nStartTime = ccCurrentTime();
	}

	float arrClip[] = { 0,0,1,1 };

	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, float(runTime));
	if (img == NULL)
	{
		return;
	}
	int dw = img->info.width;
	int dh = img->info.height;

	if (dw > 0 && dh > 0)
	{
		float fImgRatio = 1.0f*dw / dh;
		float fRTRatio = 1.0f * width / height;
		if (fImgRatio > fRTRatio)
		{
			float fClipX = fRTRatio / fImgRatio;
			fClipX = 1.0f - fClipX;
			arrClip[0] = fClipX * 0.5f;
			arrClip[2] = 1.0f - fClipX * 0.5f;
		}
		else
		{
			fImgRatio = 1.0f*dh / dw;
			fRTRatio = 1.0f*height / width;
			float fClipY = fRTRatio / fImgRatio;
			fClipY = 1.0f - fClipY;
			arrClip[1] = fClipY * 0.5f;
			arrClip[3] = 1.0f - fClipY * 0.5f;
		}
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();

	m_pShader->useShader();
	auto pMaterialView = RHIResourceCast(img->tex.get())->GetSRV();
	auto pMaskView = RHIResourceCast(BodyTexture.get())->GetSRV();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	RectConstantMask mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	mWVP.mClip = XMFLOAT4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]);
	DeviceContextPtr->UpdateSubresource(m_pConstantBufferMask, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferMask);


	//���ö�������
	unsigned int nStride = 5 * sizeof(float);
	unsigned int nOffset = 0;
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

void CBodyBGEffect::Release()
{
	SAFE_RELEASE_BUFFER(m_pConstantBufferMask);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
}

bool CBodyBGEffect::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	if (!anim)
	{
		return true;
	}
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "BodyBGEffect");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	////ʱ�����
	//XMLNode time = nodeEffect.addChild("time");


	XMLNode nodeDrwable = nodeEffect.addChild("anidrawable");
	nodeDrwable.addAttribute("name", "11");

	//get items
	if (animInfo.relative_filename_list.size() > 0)
	{
		std::string items = animInfo.relative_filename_list[0];
		items = items.replace(items.find("/") + 1, 6, "%06d");
		nodeDrwable.addAttribute("items", items.c_str());
	}
	// get iteminfo
	char iteminfo[256];
	int end = animInfo.relative_filename_list.size() - 1;
	if (animInfo.fps <= 0)
	{
		animInfo.fps = 30;
	}
	int singleFrameTime = 1000 / animInfo.fps;
	int nStep = 1;
	sprintf(iteminfo, "%d,%d,%d,%d", 0, end, singleFrameTime, nStep);
	nodeDrwable.addAttribute("iteminfo", iteminfo);

	return true;
}

