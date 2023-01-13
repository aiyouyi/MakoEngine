#include "Toolbox/DXUtils/DXUtils.h"
#include "CFaceEffect2DSticker.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "EffectKernel/VideoAnimation.h"
#include "BaseDefine/commonFunc.h"

CFaceEffect2DSticker::CFaceEffect2DSticker()
{
	m_EffectPart = FACE_EFFECT_STICKER;
	//m_drawable = NULL;
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;
	m_pMergeVertex = NULL;
	m_pConstantBuffer = NULL;
	m_alpha = 1.0;
}

CFaceEffect2DSticker::~CFaceEffect2DSticker()
{
	Release();
}

void CFaceEffect2DSticker::Release()
{

	//SAFE_DELETE(m_drawable);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}
	SAFE_DELETE_ARRAY(m_pMergeVertex);
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
}

void * CFaceEffect2DSticker::Clone()
{
	CFaceEffect2DSticker* result = new CFaceEffect2DSticker();
	*result = *this;
	return result;
}

void CFaceEffect2DSticker::Resize(int nWidth, int nHeight)
{
	if (m_Width != nWidth || m_Height != nHeight)
	{
		m_Width = nWidth;
		m_Height = nHeight;
	}
}

bool CFaceEffect2DSticker::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFaceEffect2DSticker::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL,path );
	return true;
}

bool CFaceEffect2DSticker::Prepare()
{
	//����shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
		{VERTEX_ATTRIB_TEX_COORD1, FLOAT_C2}
	};


	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	string path = m_resourcePath + "/Shader/face2dBlendMakeUp.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 3);
	
	//������
	if (b_FarModel)
	{
		m_ModelWidth = 948;
		m_ModelHeight = 533;
	}
	m_nVerts = g_TextureCoordinate.size() / 2;

	if (vdoAnimation)
	{
		vdoAnimation->Init(m_resourcePath, m_MaskVideoWidth, m_MaskVideoHeight);
	}

	return true;
}

void CFaceEffect2DSticker::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	//printf("runtime %d\n", runTime);
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

	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	m_pShader->useShader();

	float pParam[4];
	pParam[0] = m_alpha;
	pParam[1] = GetBlendParm(m_BlendType);
	if (m_EnableMp4Alpha)pParam[2] = 1.0; else pParam[2] = 0.0;


	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);


	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
	//	pDoubleBuffer->SwapFBO();
		pDoubleBuffer->BindFBOA();


		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		Matrix3 warpMat;
		//m_KeepShape = true;
		if (m_KeepShape)
		{
			float left = RenderParam.GetWidth()*2;
			float right = -1000;
			float top = RenderParam.GetHeight()*2;
			float bottom = -1000;
			float *pVerts = (float*)pFacePoint;
			for (int i = 0; i < 32; i++)
			{
				float x = pVerts[0];
				float y = pVerts[1];
				pVerts += 2;
				left = (std::min)(left, x);
				right = (std::max)(right, x);
				top = (std::min)(top, y);
				bottom = (std::max)(bottom, y);
			}

			m_DstPoint[0] = Vector2(left, top);
			m_DstPoint[1] = Vector2(left, bottom);
			m_DstPoint[2] = Vector2(right, bottom);

	
			if (b_FarModel)
			{
				float ratio = (m_FarModelSrcPoint[1].y - m_FarModelSrcPoint[0].y)/(m_FarModelSrcPoint[2].x - m_FarModelSrcPoint[1].x);
				float y_offset = (right - left) * ratio;
				m_DstPoint[1].y = m_DstPoint[0].y + y_offset;
				m_DstPoint[2].y = m_DstPoint[1].y;

				warpMat = CMathUtils::getAffineTransform(m_FarModelSrcPoint, m_DstPoint);;
			}
			else
			{
				float ratio = (m_SrcPointRect[1].y - m_SrcPointRect[0].y) / (m_SrcPointRect[2].x - m_SrcPointRect[1].x);
				float y_offset = (right - left) * ratio;
				m_DstPoint[1].y = m_DstPoint[0].y + y_offset;
				m_DstPoint[2].y = m_DstPoint[1].y;
				warpMat = CMathUtils::getAffineTransform(m_SrcPointRect, m_DstPoint);
			}
			
		}
		else
		{
			m_DstPoint[0] = pFacePoint[57];
			m_DstPoint[1] = pFacePoint[67];
			m_DstPoint[2] = pFacePoint[80];

			warpMat = CMathUtils::getAffineTransform(m_SrcPoint, m_DstPoint);
		}
		//��÷���任��������任��2*3����

		for (int i = 0; i < 4; i++)
		{
			m_SrcAbsRect[i].x = m_SrcRect[i].x * m_ModelWidth;
			m_SrcAbsRect[i].y = m_SrcRect[i].y * m_ModelHeight;
		}

		CMathUtils::transform((Vector2*)m_SrcAbsRect, (Vector2*)m_Vertices, m_nVerts, warpMat);
		
		MergeVertex((float*)m_Vertices, &g_TextureCoordinate[0], m_nVerts);
		pDoubleBuffer->SyncAToBRegion(m_pMergeVertex, m_nVerts, 5, 1);
		//printf("anim size %d\n", anim->images.size());
		ID3D11ShaderResourceView* pMaterialView = 0;
		if (AllVedioItems.size()>0)
		{
			if (CurrentVedioIndex<0)
			{
				CurrentVedioIndex= GetVideoIndex(RenderParam, runTime);
			}
			if (CurrentVedioIndex>=0 && (AllVedioItems[CurrentVedioIndex].StartTime>= runTime || AllVedioItems[CurrentVedioIndex].EndTime<=runTime))
			{
				CurrentVedioIndex = GetVideoIndex(RenderParam, runTime);
			}
			if (CurrentVedioIndex<0)
			{
				return;
			}
			if (vdoAnimation)
			{
				auto videoMaskFrame = ResourceManager::Instance().getMaskedVideo(AllVedioItems[CurrentVedioIndex].ID, double(runTime - AllVedioItems[CurrentVedioIndex].StartTime), vdoAnimation);
				if (videoMaskFrame == NULL)
				{
					return;
				}
				m_pShader->useShader();
				pDoubleBuffer->BindFBOA();
				pMaterialView = RHIResourceCast(videoMaskFrame.get())->GetSRV();
			}
			else
			{
				auto videoFrame = ResourceManager::Instance().getImageCommon(AllVedioItems[CurrentVedioIndex].ID, double(runTime- AllVedioItems[CurrentVedioIndex].StartTime));
				if (videoFrame == NULL)
				{
					return;
				}
				pMaterialView = RHIResourceCast(videoFrame->tex.get())->GetSRV();
			}
		}
		else
		{
			Image* img = ResourceManager::Instance().getImageCommon(m_anim_id, float(runTime));
			if (img == NULL)
			{
				return;
			}
			pMaterialView = RHIResourceCast(img->tex.get())->GetSRV();
		}
		auto pSrcShaderView = RHIResourceCast(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB().get())->GetSRV();
		DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
		DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		if (m_IndexBuffer == NULL)
		{
			unsigned short index[] =
			{
				0, 1, 2,
				1, 2, 3
			};
			m_IndexBuffer = DXUtils::CreateIndexBuffer(index, 2);
		}
		if (m_VerticeBuffer[faceIndex] == NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, m_nVerts, 5);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, m_nVerts, 5 * sizeof(float), 5 * sizeof(float));
		}
		unsigned int nStride = (3 + 2) * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	}
}

void CFaceEffect2DSticker::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{

		XMLNode nodeRect = childNode.getChildNode("rect", 0);
		if (!nodeRect.isEmpty())
		{
			const char* szLeft = nodeRect.getAttribute("left");
			const char* szUp = nodeRect.getAttribute("up");
			const char* szRight = nodeRect.getAttribute("right");
			const char* szDown = nodeRect.getAttribute("down");
			if (szLeft != NULL && szUp != NULL && szRight != NULL && szDown != NULL)
			{
				float left = atof(szLeft);
				float up = atof(szUp);
				float right = atof(szRight);
				float down = atof(szDown);

				m_SrcRect[0] = Vector2(left, down);
				m_SrcRect[1] = Vector2(right, down);
				m_SrcRect[2] = Vector2(left, up);
				m_SrcRect[3] = Vector2(right, up);
			}


			const char* szShape = nodeRect.getAttribute("keepShpape");
			if (szShape!=NULL)
			{
				m_KeepShape = atoi(szShape);
			}
	
		}
		//改用point字段
		XMLNode nodePoint = childNode.getChildNode("point", 0);
		if (!nodePoint.isEmpty())
		{
			
			const char* szLeftDown = nodePoint.getAttribute("leftdown");
			const char* szRightDown = nodePoint.getAttribute("rightdown");
			const char* szLeftUp = nodePoint.getAttribute("leftup");
			const char* szRightUp = nodePoint.getAttribute("rightup");
			if (szLeftDown != NULL && szRightDown != NULL && szLeftUp != NULL && szRightUp != NULL)
			{
				sscanf(szLeftDown, "%f,%f", &m_SrcRect[0].x, &m_SrcRect[0].y);
				sscanf(szRightDown, "%f,%f", &m_SrcRect[1].x, &m_SrcRect[1].y);
				sscanf(szLeftUp, "%f,%f", &m_SrcRect[2].x, &m_SrcRect[2].y);
				sscanf(szRightUp, "%f,%f", &m_SrcRect[3].x, &m_SrcRect[3].y);
			}
		
			const char* szShape = nodePoint.getAttribute("keepShpape");
			if (szShape != NULL)
			{
				m_KeepShape = atoi(szShape);
			}

			const char* szFarModel = nodePoint.getAttribute("EnableFarModel");
			if (szFarModel != NULL && !strcmp(szFarModel, "true"))
			{
				b_FarModel = true;
			}

		}

		//Time
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

		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip==0)
				{
					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					m_anim_id = ResourceManager::Instance().loadAnim(anim_info);

					//m_drawable = new BitmapDrawable(pTex);
				}
				else
				{
					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
					//m_drawable = new BitmapDrawable(pTex);
				}

				const char *szBlendType = nodeDrawable.getAttribute("blendType");
				m_BlendType = GetBlendType(szBlendType);

			}

		}

		//anidrawable 
		nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");

			//AnimationDrawable *drawable = NULL;
			//
			//if (drawable == NULL)
			//{
			//	drawable = new AnimationDrawable();
			//}

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char *szOffset = nodeDrawable.getAttribute("offset");
			if (szOffset != NULL)
			{
				long offset = atol(szOffset);
				//drawable->setOffset(offset);
				m_anim_offset = offset;
			}
			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
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

						if (hZip==0)
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
							anim_info.relative_filename_list.push_back(szImagePath);

							//DX11Texture *pTex = ContextInst->fetchTexture(szFullFile);
							//if (pTex == NULL)
							//{
							//	pTex = new DX11Texture();
							//	pTex->initTextureFromFileCPUAcess(szFullFile);
							//	ContextInst->recordTexture(szFullFile, pTex);
							//}
							//long during = nDuring;

							//drawable->appandTex(during, pTex);
							//if (pTex != NULL)
							//{
							//	pTex->unref();
							//}
						}
						else
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
							anim_info.relative_filename_list.push_back(szImagePath);
						}
						//else
						//{
						//	sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);
						//	DX11Texture *pTex = ContextInst->fetchTexture(szFullFile);
						//	if (pTex == NULL)
						//	{
						//		pTex = new DX11Texture();
						//		pTex->initTextureFromZip(hZip, szImagePath, bGenMipmap);
						//		ContextInst->recordTexture(szFullFile, pTex);
						//	}
						//	long during = nDuring;

						//	drawable->appandTex(during, pTex);
						//	if (pTex != NULL)
						//	{
						//		pTex->unref();
						//	}
						//}
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

			//m_drawable = drawable;
		}

		nodeDrawable = childNode.getChildNode("webpdrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			const char* szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					//To do

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

		int VideoIndex = 0;
		XMLNode nodeVideo = childNode.getChildNode("video", VideoIndex);
		while (!nodeVideo.isEmpty())
		{
			const char* szBlendType = nodeVideo.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char* szAlpha = nodeVideo.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}

			StickerVedioInfo Vinfo;
			const char* TempChar= nodeVideo.getAttribute("Material");
			if (TempChar != NULL)
			{
				Vinfo.Path = TempChar;
			}
			

			TempChar = nodeVideo.getAttribute("start");
			if (TempChar != NULL)
			{
				sscanf(TempChar, "%d", &Vinfo.StartFrameTime);
			}
			

			TempChar = nodeVideo.getAttribute("end");
			if (TempChar != NULL)
			{
				sscanf(TempChar, "%d", &Vinfo.EndFrameTime);
			}
			TempChar = nodeVideo.getAttribute("condition");
			if (TempChar != NULL)
			{
				sscanf(TempChar, "%d", &Vinfo.Condition);
			}

			if (hZip)
			{
				Vinfo.Info.fps = 0;
				Vinfo.Info.dir = path;
				Vinfo.Info.relative_filepath= Vinfo.Path;
				Vinfo.ID = ResourceManager::Instance().loadVideoFromZip(Vinfo.Info, hZip);

				m_MaskVideoWidth = Vinfo.Info.width*0.5;
				m_MaskVideoHeight = Vinfo.Info.height;

				Vinfo.StartTime = Vinfo.StartFrameTime * 1000.0 / Vinfo.Info.fps;
				Vinfo.EndTime = Vinfo.EndFrameTime * 1000.0 / Vinfo.Info.fps;
				AllVedioItems.push_back(Vinfo);

			}

			const char* maskJson = nodeVideo.getAttribute("maskJson");
			if (maskJson != NULL)
			{
				m_EnableMaskJson = true;
				vdoAnimation = std::make_shared<VideoAnimation>();
				std::string jsonName("a.json");
				vdoAnimation->ReadJsonFromZip(jsonName, hZip);
			}
			const char* szMasktype = nodeVideo.getAttribute("maskType");
			if (szMasktype != NULL)
			{
				std::vector<std::string> str;
				StringSplit(szMasktype, ',', str);
				m_AnchorType.resize(str.size());
				for (int i = 0; i < str.size(); i++)
				{
					m_AnchorType[i] = AnchorType(atoi(str[i].c_str()));
				}
			}
			const char* szEnableMp4Alpha = nodeVideo.getAttribute("EnableMp4Alpha");
			if (szEnableMp4Alpha != NULL && !strcmp(szEnableMp4Alpha, "true"))
			{
				m_EnableMp4Alpha = true;
			}

			nodeVideo = childNode.getChildNode("video", ++VideoIndex);
		}
	}
}

bool CFaceEffect2DSticker::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "FaceEffect2DSticker");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	////时间控制
	//XMLNode time = nodeEffect.addChild("time");

	XMLNode nodePoint = nodeEffect.addChild("point");
	char point[256];
	sprintf(point, "%.4f,%.4f", m_SrcRect[2].x, m_SrcRect[2].y);
	nodePoint.addAttribute("leftup", point);

	sprintf(point, "%.4f,%.4f", m_SrcRect[3].x, m_SrcRect[3].y);
	nodePoint.addAttribute("rightup", point);

	sprintf(point, "%.4f,%.4f", m_SrcRect[1].x, m_SrcRect[1].y);
	nodePoint.addAttribute("rightdown", point);

	sprintf(point, "%.4f,%.4f", m_SrcRect[0].x, m_SrcRect[0].y);
	nodePoint.addAttribute("leftdown", point);

	nodePoint.addAttribute("keepShpape", std::to_string(int(m_KeepShape)).c_str());


	XMLNode nodeDrwable = nodeEffect.addChild("anidrawable");
	nodeDrwable.addAttribute("name","11");

	nodeDrwable.addAttribute("offset", "0");

	char alpha[256];
	sprintf(alpha, "%.4f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	GetBlendParm(m_BlendType);
	nodeDrwable.addAttribute("blendType", m_BlendName.c_str());
	//get items
	if (animInfo.relative_filename_list.size() > 0)
	{
		std::string items = animInfo.relative_filename_list[0];
		items = items.replace(items.find("/")+1, 6, "%06d");
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

void CFaceEffect2DSticker::MergeVertex(float * pVertex, float * pUV,int nVertex)
{
	Vector2 ScaleSize(1.0 / m_Width, 1.0 / m_Height);
	
	if (m_pMergeVertex == NULL)
	{
		int nSize = nVertex;
		m_pMergeVertex = new float[nSize*(3 + 2)];
	}
	for (int i = 0; i < nVertex; i++)
	{
		m_pMergeVertex[i * 5] = pVertex[i * 2] * ScaleSize.x *2.0 - 1.0;
		m_pMergeVertex[i * 5 + 1] = pVertex[i * 2 + 1] * ScaleSize.y *2.0 - 1.0;
		m_pMergeVertex[i * 5 + 2] = 0.5;
		m_pMergeVertex[i * 5 + 3] = pUV[i * 2];
		m_pMergeVertex[i * 5 + 4] = 1.0 - pUV[i * 2 + 1];
	}
}

int CFaceEffect2DSticker::GetVideoIndex(BaseRenderParam& RenderParam, long Rumtime)
{
	FacePosInfo* FaceInfo = RenderParam.GetFaceInfo(0);
	for (int Index = 0; Index < AllVedioItems.size(); Index++)
	{
		if (AllVedioItems[Index].StartTime<= Rumtime&& AllVedioItems[Index].EndTime>=Rumtime)
		{
			if (FaceConditionMatch(AllVedioItems[Index].Condition, FaceInfo))
			{
				return Index;
			}
		}
	}
	return -1;
}

bool CFaceEffect2DSticker::FaceConditionMatch(FaceBodyCondition Condition, FacePosInfo* FaceInfo)
{
	if (FaceInfo)
	{
		switch (Condition)
		{
		case None:
			return true;
		case MouthOpen:
			return FaceInfo->pFaceExp ? FaceInfo->pFaceExp->openMouse : false;
		case MouthClose:
			return FaceInfo->pFaceExp ? (!FaceInfo->pFaceExp->openMouse) : false;
		default:
			break;
		}
	}
	return false;
}
