#include "CFaceEffect2D.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Algorithm/Matrices.h"
#include "Algorithm/MathUtils.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"

CFaceEffect2D::CFaceEffect2D()
{
	m_EffectPart = FACE_UV_MASK;

	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_2DInterFace = NULL;
	m_pMergeVertex = NULL;
}


CFaceEffect2D::~CFaceEffect2D()
{
	Release();
}

void CFaceEffect2D::Release()
{
	SAFE_DELETE(m_2DInterFace);
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}
	SAFE_DELETE_ARRAY(m_pMergeVertex);

	ResourceManager::Instance().freeAnim(m_MaskId);
}

void * CFaceEffect2D::Clone()
{
	CFaceEffect2D* result = new CFaceEffect2D();
	*result = *this;
	return result;
}

bool CFaceEffect2D::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFaceEffect2D::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

long long CFaceEffect2D::GetImageID(HZIP hZip, const std::string & path, const char *szDrawableName)
{
	long long m_anim_id = 0;

	if (hZip == 0)
	{

		AnimInfo anim_info;
		anim_info.fps = 0;
		anim_info.dir = path;
		anim_info.relative_filename_list.push_back(szDrawableName);
		m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
	}
	else
	{
		AnimInfo anim_info;
		anim_info.fps = 0;
		anim_info.dir = path;
		anim_info.relative_filename_list.push_back(szDrawableName);
		m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
	}
	return m_anim_id;
}

void CFaceEffect2D::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			m_AffineUV = false;
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			
			if (szDrawableName != NULL)
			{
				//std::shared_ptr<CC3DTextureRHI> TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szDrawableName, false);
				//m_FaceEffectInfo->m_drawable = new BitmapDrawable(TexRHI);
				m_anim_id = GetImageID(hZip, path, szDrawableName);
			}
			//mask 创建对应的mask Id
			const char *szmask = nodeDrawable.getAttribute("Mask");
			if (szmask != NULL)
			{
				//m_FaceEffectInfo->m_mask =  DXUtils::CreateTexFromZIP(hZip, szmask, false);
				m_MaskId = GetImageID(hZip, path, szmask);
			}
			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
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
		//改用point字段
		XMLNode nodePoint = childNode.getChildNode("point", 0);
		if (!nodePoint.isEmpty())
		{
			m_AffineUV = true;
			const char* szLeftDown = nodePoint.getAttribute("leftdown");
			const char* szRightDown = nodePoint.getAttribute("rightdown");
			const char* szLeftUp = nodePoint.getAttribute("leftup");
			const char* szRightUp = nodePoint.getAttribute("rightup");
			if (szLeftDown != NULL && szRightDown != NULL && szLeftUp != NULL && szRightUp != NULL)
			{
				sscanf(szLeftUp, "%f,%f", &m_SrcRect[0].x, &m_SrcRect[0].y);
				sscanf(szRightUp, "%f,%f", &m_SrcRect[1].x, &m_SrcRect[1].y);
				sscanf(szRightDown, "%f,%f", &m_SrcRect[2].x, &m_SrcRect[2].y);
				sscanf(szLeftDown, "%f,%f", &m_SrcRect[3].x, &m_SrcRect[3].y);
			}

		}
		//anidrawable 
		nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char *szmask = nodeDrawable.getAttribute("Mask");	
			if (szmask != NULL)
			{
				//m_FaceEffectInfo->m_mask = DXUtils::CreateTexFromZIP(hZip, szmask, false);
				m_MaskId = GetImageID(hZip, path, szmask);
			}

			const char *szMixType = nodeDrawable.getAttribute("mixtype");
			if (szMixType != NULL)
			{
				int type;
				sscanf(szMixType, "%d", &type);
				m_MixType = MixType(type);
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
		XMLNode nodeparam = childNode.getChildNode("param", 0);
		if (!nodeparam.isEmpty())
		{
			const char *szMixType = nodeparam.getAttribute("mixtype");
			if (szMixType != NULL)
			{
				int type;
				sscanf(szMixType, "%d", &type);
				m_MixType = MixType(type);
			}
		}
	}
}

bool CFaceEffect2D::Prepare()
{
	//创建shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
		{VERTEX_ATTRIB_TEX_COORD1, FLOAT_C2}
	};

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 8);
	}

	std::string path = m_resourcePath + "/Shader/face2dEffect.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 3);
	m_nVertsSize = 7;
	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceEffect2D::Render(BaseRenderParam &RenderParam)
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
	//DXUtils::SetRasterizerState(false);
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();

	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	float pParam[8];
	pParam[0] = m_alpha;
	pParam[1] = GetBlendParm(m_BlendType);

	if (m_AffineUV)
	{
		for (int i = 0; i < 3; i++)
		{
			m_SrcPoint[i] = m_SrcRect[i];
		}
		Matrix3 warpMat = CMathUtils::getAffineTransform(m_SrcPoint, m_DstPoint);
		memcpy(pParam + 2, (float*)warpMat.get(), 6 * sizeof(float));
	}
	else
	{
		for (int i = 0; i < 6; i++)
		{
			pParam[i + 2] = 0.0;
		}
		pParam[2] = 1.0;
		pParam[6] = 1.0;
	}

	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, float(runTime));
	if (img == NULL)
	{
		return;
	}
	auto pMaterialView = RHIResourceCast(img->tex.get())->GetSRV();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);

	img = ResourceManager::Instance().getAnimFrame(m_MaskId,0);
	if (img == NULL)
	{
		return;
	}
	auto pMaskView = RHIResourceCast(img->tex.get())->GetSRV();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);

	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);

		auto FaceInfo = RenderParam.GetFaceInfo(faceIndex);

		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), FaceInfo->pitch, mt3dface::MT_FACE_25D);
		float *pMaskUV = m_2DInterFace->GetStandVerts();
		MergeVertex(FaceMesh->pVertexs, FaceMesh->pTextureCoordinates, pMaskUV, FaceMesh->nVertex);

		pDoubleBuffer->SyncAToBRegion((float*)FaceMesh->pVertexs, FaceMesh->nVertex, 3, 1);

		//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(2, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(2, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());

		if (m_IndexBuffer == NULL)
		{
			m_IndexBuffer = DXUtils::CreateIndexBuffer(FaceMesh->pTriangleIndex, FaceMesh->nTriangle);
		}
		if (m_VerticeBuffer[faceIndex] == NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, FaceMesh->nVertex, m_nVertsSize);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, FaceMesh->nVertex, m_nVertsSize * sizeof(float), m_nVertsSize * sizeof(float));
		}
		unsigned int nStride = m_nVertsSize * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(FaceMesh->nTriangle * 3, 0, 0);
	}
//	DXUtils::SetRasterizerState(true);
}

void CFaceEffect2D::MergeVertex(float * pVertex, float * pUV, float *pMaskUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		m_pMergeVertex = new float[nVertex*m_nVertsSize];
	}
	for (int i = 0;i<nVertex;i++)
	{
		m_pMergeVertex[i *m_nVertsSize] = pVertex[i*3];
		m_pMergeVertex[i * m_nVertsSize + 1] = pVertex[i * 3 + 1];
		m_pMergeVertex[i * m_nVertsSize + 2] = pVertex[i * 3 + 2];
		m_pMergeVertex[i * m_nVertsSize + 3] = pUV[i * 2];
		m_pMergeVertex[i * m_nVertsSize + 4] = pUV[i * 2 + 1];
		m_pMergeVertex[i * m_nVertsSize + 5] = pMaskUV[i * 2];
		m_pMergeVertex[i * m_nVertsSize + 6] = pMaskUV[i * 2 + 1];
	}
}

bool CFaceEffect2D::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	//mask img save
	anim = ResourceManager::Instance().getAnim(m_MaskId);
	AnimInfo animInfoMask= anim->info;
	renamed_info = FileManager::Instance().AddAnim(animInfoMask);
	animInfoMask.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "FaceEffect2D");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	////时间控制
	//XMLNode time = nodeEffect.addChild("time");
	XMLNode nodePoint = nodeEffect.addChild("point");
	char point[256];
	sprintf(point, "%.4f,%.4f", m_SrcRect[0].x, m_SrcRect[0].y);
	nodePoint.addAttribute("leftup", point);

	sprintf(point, "%.4f,%.4f", m_SrcRect[1].x, m_SrcRect[1].y);
	nodePoint.addAttribute("rightup", point);

	sprintf(point, "%.4f,%.4f", m_SrcRect[2].x, m_SrcRect[2].y);
	nodePoint.addAttribute("rightdown", point);

	sprintf(point, "%.4f,%.4f", m_SrcRect[3].x, m_SrcRect[3].y);
	nodePoint.addAttribute("leftdown", point);

	XMLNode nodeDrwable = nodeEffect.addChild("anidrawable");
	nodeDrwable.addAttribute("name", "11");

	char alpha[256];
	sprintf(alpha, "%.4f",m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	sprintf(alpha, "%d", m_MixType);
	nodeDrwable.addAttribute("mixtype", alpha);

	GetBlendParm(m_BlendType);
	nodeDrwable.addAttribute("blendType", m_BlendName.c_str());


	
	//get items
	if (animInfoMask.relative_filename_list.size() > 0)
	{
		std::string items = animInfoMask.relative_filename_list[0];
		nodeDrwable.addAttribute("Mask", items.c_str());
	}
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