#include "CBodyTrack2DEffect.h"
#include "common.h"
#include <xnamath.h>
#include <BaseDefine/Vectors.h>
#include "Algorithm/MathUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "EffectKernel/VideoAnimation.h"
#include "BaseDefine/commonFunc.h"

struct RectConstantMaskCT
{
	XMMATRIX mWVP; //混合矩阵
	XMFLOAT4 mClip;//背景裁剪区域
	float splitNum;
	float currentSplit;
	XMFLOAT2 Affine0_1;
	XMFLOAT4 Affine2_5;
	XMFLOAT2 WithHeight;
	float Alpha;
	float Mirror;
	XMFLOAT4 blendParam;
};

CBodyTrack2DEffect::CBodyTrack2DEffect()
{
	m_EffectPart = BODY_TRACK2D_EFFECT;
	m_nEffectDuring = -1;
	m_nAniLoopTime = -1;
	m_nStartTime = -1;
	m_nLoopStartTime = -1;
	m_alpha = 1.0;

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBufferMask = NULL;

	SrcPoint[0] = Vector2(402,196);
	SrcPoint[1] = Vector2(478, 39);
	SrcPoint[2] = Vector2(548, 206);
	for (size_t i = 0; i < 3; i++)
	{
		SrcPoint[i].x = SrcPoint[i].x *2/ 948 - 1;
		SrcPoint[i].y = SrcPoint[i].y *2/ 533 - 1;
		SrcPoint[i].y *= -1;
	}
}


CBodyTrack2DEffect::~CBodyTrack2DEffect()
{
	
	Release();
}

void * CBodyTrack2DEffect::Clone()
{
	CBodyTrack2DEffect* result = new CBodyTrack2DEffect();
	*result = *this;
	return result;
}

bool CBodyTrack2DEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CBodyTrack2DEffect::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CBodyTrack2DEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* szDrawableName = nodeDrawable.getAttribute("name");

			const char *isTrack = nodeDrawable.getAttribute("Track");
			if (isTrack != NULL && !strcmp(isTrack, "false"))
			{
				m_IsTrack = false;
			}
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

		nodeDrawable = childNode.getChildNode("webpdrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *isTrack = nodeDrawable.getAttribute("Track");
			if (isTrack != NULL && !strcmp(isTrack, "false"))
			{
				m_IsTrack = false;
			}
			const char* szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			const char* szRoate = nodeDrawable.getAttribute("DisableRoate");
			if (szRoate != NULL && !strcmp(szRoate, "true"))
			{
				m_DisableRoate = true;
			}
			const char* szRoateValue = nodeDrawable.getAttribute("RoateValue");
			if (szRoateValue != NULL)
			{
				m_RoateValue = atof(szRoateValue);
			}
			const char* szEnableMp4Alpha = nodeDrawable.getAttribute("EnableMp4Alpha");
			if (szEnableMp4Alpha != NULL && !strcmp(szEnableMp4Alpha, "true"))
			{
				m_EnableMp4Alpha = true;
			}
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					WebpAnimInfo webp_info;
					webp_info.webp_dir = path;
					webp_info.webp_relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadWebpFromFile(webp_info);

				}
				else
				{
					WebpAnimInfo webp_info;
					webp_info.webp_dir = path;
					webp_info.webp_relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadWebpFromZip(webp_info, hZip);
				}

			}
		}

		nodeDrawable = childNode.getChildNode("videodrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* isTrack = nodeDrawable.getAttribute("Track");
			if (isTrack != NULL && !strcmp(isTrack, "false"))
			{
				m_IsTrack = false;
			}
			m_VideoXMLInfo.Track = isTrack ? isTrack : "";

			const char* szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
			m_VideoXMLInfo.blendType = szBlendType;

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			const char* szRoate = nodeDrawable.getAttribute("DisableRoate");
			if (szRoate != NULL && !strcmp(szRoate, "true"))
			{
				m_DisableRoate = true;
			}

			const char* szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				m_VideoXMLInfo.Material = szDrawableName;
				if (hZip == 0)
				{
					VideoInfo video_info;
					video_info.dir = path;
					video_info.relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadVideo(video_info);

					m_MaskVideoWidth = video_info.width * 0.5;
					m_MaskVideoHeight = video_info.height;
				}
				else
				{
					VideoInfo video_info;
					video_info.dir = path;
					video_info.relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadVideoFromZip(video_info, hZip);
					
					m_MaskVideoWidth = video_info.width * 0.5;
					m_MaskVideoHeight = video_info.height;
				}
			}

			const char* maskJson = nodeDrawable.getAttribute("maskJson");
			if (maskJson != NULL)
			{
				m_EnableMaskJson = true;
				vdoAnimation = std::make_shared<VideoAnimation>();
				std::string jsonName("a.json");
				if (hZip == 0)
				{
					jsonName = path + "/" + jsonName;
					vdoAnimation->ReadJsonFromFile(jsonName);
				}
				else
				{
					vdoAnimation->ReadJsonFromZip(jsonName, hZip);
				}
				
			}
			const char* szMasktype = nodeDrawable.getAttribute("maskType");
			if (szMasktype != NULL)
			{
				std::vector<std::string> str;
				StringSplit(szMasktype, ',', str);
				m_AnchorType.resize(str.size());
				for (int i = 0; i < str.size();i++)
				{
					m_AnchorType[i] = AnchorType(atoi(str[i].c_str()));
				}
			}
			const char* szEnableMp4Alpha = nodeDrawable.getAttribute("EnableMp4Alpha");
			if (szEnableMp4Alpha != NULL && !strcmp(szEnableMp4Alpha, "true"))
			{
				m_EnableMp4Alpha = true;
			}
			m_VideoXMLInfo.EnableMp4Alpha = szEnableMp4Alpha ? szEnableMp4Alpha : "";
		}
	}
	return false;
}

bool CBodyTrack2DEffect::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	string path = m_resourcePath + "/Shader/CBodyTrack2DEffect.fx";
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

	m_pConstantBufferMask = DXUtils::CreateConstantBuffer(sizeof(RectConstantMaskCT));

	if (vdoAnimation)
	{
		vdoAnimation->Init(m_resourcePath, m_MaskVideoWidth, m_MaskVideoHeight);
	}
	return true;
}

void CBodyTrack2DEffect::Render(BaseRenderParam & RenderParam)
{
	//return;
	auto BodyTexture = RenderParam.GetBodyMaskTexture();
	if (BodyTexture == NULL)
	{
		return;
	}

	bool bMirror;
	int SplitScreenNum = RenderParam.GetSplitScreenNum(bMirror);

	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	if (vdoAnimation && m_EnableMaskJson)
	{
		std::map<AnchorType, long long> mCardId = RenderParam.GetCardMaskID();
		for (int index = 0; index < m_AnchorType.size(); index++)
		{
			auto iter = mCardId.find(m_AnchorType[index]);
			if (iter != mCardId.end())
			{
				vdoAnimation->SetTexture(index, iter->second);
			}		
		}
		m_EnableMaskJson = false;
	}

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	
	if (m_IsTrack)
	{
		if (RenderParam.GetBodyCount() <= 0)
		{
			return;
		}
	}
	else//无跟随的分屏处理
	{
		if (SplitScreenNum == 1.0)
		{
			m_DstVertexPoint[0][0] = Vector2(0.0, 0.0) * 2.0 - 1.0;
			m_DstVertexPoint[0][1] = Vector2(0.0, 1.0) * 2.0 - 1.0;
			m_DstVertexPoint[0][2] = Vector2(1.0, 1.0) * 2.0 - 1.0;
		}

		else if (SplitScreenNum == 2.0)
		{
			m_DstVertexPoint[0][0] = Vector2(-0.25, 0.0) * 2.0 - 1.0;
			m_DstVertexPoint[0][1] = Vector2(-0.25, 1.0) * 2.0 - 1.0;
			m_DstVertexPoint[0][2] = Vector2(0.75, 1.0) * 2.0 - 1.0;

			m_DstVertexPoint[1][0] = Vector2(0.25, 0.0) * 2.0 - 1.0;
			m_DstVertexPoint[1][1] = Vector2(0.25, 1.0) * 2.0 - 1.0;
			m_DstVertexPoint[1][2] = Vector2(1.25, 1.0) * 2.0 - 1.0;
		}
		else if (SplitScreenNum == 3.0)
		{
			m_DstVertexPoint[0][0] = Vector2(-1 / 3.0, 0.0) * 2.0 - 1.0;
			m_DstVertexPoint[0][1] = Vector2(-1 / 3.0, 1.0) * 2.0 - 1.0;
			m_DstVertexPoint[0][2] = Vector2(1 / 3.0*2, 1.0) * 2.0 - 1.0;

			m_DstVertexPoint[1][0] = Vector2(0.0, 0.0) * 2.0 - 1.0;
			m_DstVertexPoint[1][1] = Vector2(0.0, 1.0) * 2.0 - 1.0;
			m_DstVertexPoint[1][2] = Vector2(1.0, 1.0) * 2.0 - 1.0;

			m_DstVertexPoint[2][0] = Vector2(1 / 3.0, 0.0) * 2.0 - 1.0;
			m_DstVertexPoint[2][1] = Vector2(1 / 3.0, 1.0) * 2.0 - 1.0;
			m_DstVertexPoint[2][2] = Vector2(1.0 + 1 / 3.0, 1.0) * 2.0 - 1.0;
		}
	}
	if (m_nStartTime < 0)
	{
		m_nLoopStartTime = m_nStartTime = ccCurrentTime();
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();

	m_pShader->useShader();
	
	ID3D11ShaderResourceView* pMaterialView = 0;
	int dw = 0, dh = 0;
	if (vdoAnimation)
	{
		auto videoMaskFrame = ResourceManager::Instance().getMaskedVideo(m_anim_id, double(runTime), vdoAnimation);
		if (videoMaskFrame == NULL)
		{
			return;
		}
		m_pShader->useShader();
		pDoubleBuffer->BindFBOA();
		pMaterialView = RHIResourceCast(videoMaskFrame.get())->GetSRV();

		dw = videoMaskFrame->GetWidth();
		dh = videoMaskFrame->GetHeight();
	}
	else
	{
		Image* img = ResourceManager::Instance().getImageCommon(m_anim_id, double(runTime));
		if (img == NULL)
		{
			return;
		}
		pMaterialView = RHIResourceCast(img->tex.get())->GetSRV();

		dw = img->info.width;
		dh = img->info.height;
	}


	float arrClip[] = { 0,0,1,1 };
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


	auto pMaskView = RHIResourceCast(BodyTexture.get())->GetSRV();
	auto pSrcMaterView = RHIResourceCast(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB().get())->GetSRV();

	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);
	DeviceContextPtr->PSSetShaderResources(2, 1, &pSrcMaterView);
	
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	RectConstantMaskCT mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	mWVP.mClip = XMFLOAT4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]);
	mWVP.splitNum = SplitScreenNum;
	mWVP.Mirror = bMirror ? 1.0 : 0;
	mWVP.WithHeight.x = width;
	mWVP.WithHeight.y = height;

	mWVP.blendParam.x = m_alpha;
	mWVP.blendParam.y = GetBlendParm(m_BlendType);
	if (m_EnableMp4Alpha)mWVP.blendParam.z = 1.0; else mWVP.blendParam.z = 0.0;

	static const float LerpK = 0.3;
	for (int i = 0; i < mWVP.splitNum; i++)
	{
		mWVP.currentSplit = i;
		
		//是否肢体跟随
		if (m_IsTrack)
		{
			int BodyIndex = GetBodyIndex(RenderParam, mWVP.splitNum, i);
			if (BodyIndex < 0)continue;

			auto BodyPoint = RenderParam.GetBodyPoint(BodyIndex);
			Vector2 DesPoint[3];
			DesPoint[0] = BodyPoint[12];
			DesPoint[1] = BodyPoint[9];
			DesPoint[2] = BodyPoint[13];
			auto Delta = DesPoint[2] - DesPoint[0];
			Delta.x /= width;
			Delta.y /= height;
			for (size_t index = 0; index < 3; index++)
			{
				if (bMirror&& i != 0)
				{
					DesPoint[index].x = width / mWVP.splitNum - DesPoint[index].x;
				}
				DesPoint[index].x += width / mWVP.splitNum * i;
				DesPoint[index].x = DesPoint[index].x * 2 / width - 1;
				DesPoint[index].y = DesPoint[index].y * 2 / height - 1;
				DesPoint[index].y *= -1;

				DesPoint[index].x = DesPoint[index].x * LerpK + LastDes[i][index].x * (1 - LerpK);
				DesPoint[index].y = DesPoint[index].y * LerpK + LastDes[i][index].y * (1 - LerpK);

				LastDes[i][index] = DesPoint[index];
			}
			auto AffineMat = CMathUtils::getAffineTransform(SrcPoint, DesPoint);
			if (m_DisableRoate)
			{
				AffineMat[1] = AffineMat[1] * m_RoateValue;
				AffineMat[3] = AffineMat[3] * m_RoateValue;

 				float scale_x = std::sqrt(AffineMat[0] * AffineMat[0] + AffineMat[3] * AffineMat[3]);
 				float scale_y = (AffineMat[0] * AffineMat[4] - AffineMat[1] * AffineMat[3]) / scale_x;
 
 				AffineMat[0] = scale_x;
 				AffineMat[4] = scale_y;
			}
			float A = Delta.length() > 0.07 ? 1 : 0;
			LastAlpha = mWVP.Alpha = LastAlpha * (1 - LerpK) + A * LerpK;
			memcpy(&mWVP.Affine0_1, (float*)AffineMat.get(), 6 * sizeof(float));
		}
		else
		{
			auto warpMat = CMathUtils::getAffineTransform(m_SrcVertexPoint, m_DstVertexPoint[i]);
			mWVP.Alpha = 1.0;
			memcpy(&mWVP.Affine0_1, (float*)warpMat.get(), 6 * sizeof(float));
		}

		DeviceContextPtr->UpdateSubresource(m_pConstantBufferMask, 0, NULL, &mWVP, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferMask);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBufferMask);

		//设置顶点数据
		unsigned int nStride = 5 * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	}

	//pDoubleBuffer->SwapFBO();

}

void CBodyTrack2DEffect::Release()
{
	SAFE_RELEASE_BUFFER(m_pConstantBufferMask);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
}

bool CBodyTrack2DEffect::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);


	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "BodyTrack2DEffect");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	////时间控制
	//XMLNode time = nodeEffect.addChild("time");

	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	if (anim)
	{
		AnimInfo animInfo = anim->info;
		AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
		animInfo.relative_filename_list = renamed_info.relative_filename_list;

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
	}
	else
	{
		if (m_VideoXMLInfo.Material.empty())
		{
		}
		else
		{
			XMLNode nodeDrwable = nodeEffect.addChild("videodrawable");
			nodeDrwable.addAttribute("Material", m_VideoXMLInfo.Material.c_str());
			if (!m_VideoXMLInfo.Track.empty())
			{
				nodeDrwable.addAttribute("Track", m_VideoXMLInfo.Track.c_str());
			}
			
			nodeDrwable.addAttribute("blendType", m_VideoXMLInfo.blendType.c_str());
			if (!m_VideoXMLInfo.EnableMp4Alpha.empty())
			{
				nodeDrwable.addAttribute("EnableMp4Alpha", m_VideoXMLInfo.EnableMp4Alpha.c_str());
			}
			
		}
	}

	return true;
}

int CBodyTrack2DEffect::GetBodyIndex(BaseRenderParam& RenderParam, int SplitNum, int SplitIndex)
{
	return 0;
}

