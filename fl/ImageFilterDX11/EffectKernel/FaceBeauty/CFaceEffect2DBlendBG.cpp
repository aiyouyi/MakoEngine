#include "CFaceEffect2DBlendBG.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"
//#include "opencv2/opencv.hpp"



CFaceEffect2DBlendBG::CFaceEffect2DBlendBG()
{
	m_isFirstInit = true;
	for (int i=0;i<MAX_SUPPORT_PEOPLE;i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_2DInterFace = NULL;
	m_pMergeVertex = NULL;
	m_alpha = 1.0;
	m_EffectPart = FACE_MASK;
}


CFaceEffect2DBlendBG::~CFaceEffect2DBlendBG()
{
	Release();
}

void CFaceEffect2DBlendBG::Release()
{
	SAFE_DELETE(m_2DInterFace);
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}

	SAFE_DELETE_ARRAY(m_pMergeVertex);
}


void * CFaceEffect2DBlendBG::Clone()
{
	CFaceEffect2DBlendBG* result = new CFaceEffect2DBlendBG();
	*result = *this;
	return result;
}

bool CFaceEffect2DBlendBG::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFaceEffect2DBlendBG::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CFaceEffect2DBlendBG::Prepare()
{
	//videoFrame->tex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, video_width, video_height);
	//����shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/face2dBlendMakeUp.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);
	DXUtils::CompileShaderWithFile(m_pShader, (char*)path.c_str(), pAttribute, 2);

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceEffect2DBlendBG::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	if (RenderParam.GetFaceCount()==0)
	{
		return;
	}
	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	//pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();

	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);


	float pParam[4];
	pParam[0] = m_alpha;
	pParam[1] = GetBlendParm(m_BlendType);

	//Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, float(runTime));
	//auto pMaterialView = img->tex->getTexShaderView();


	auto videoFrame = ResourceManager::Instance().getImageCommon(m_anim_id, double(runTime));
	auto pMaterialView = videoFrame->tex->getTexShaderView();

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);



	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
		MergeVertex(FaceMesh->pVertexs, FaceMesh->pTextureCoordinates, FaceMesh->nVertex);

		pDoubleBuffer->SyncAToBRegion((float*)FaceMesh->pVertexs, FaceMesh->nVertex,3,1);

		auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);

		if (m_IndexBuffer == NULL)
		{
			m_IndexBuffer = DXUtils::CreateIndexBuffer(FaceMesh->pTriangleIndex, FaceMesh->nTriangle);
		}
		if (m_VerticeBuffer[faceIndex] == NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, FaceMesh->nVertex, 5);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, FaceMesh->nVertex, 5 * sizeof(float), 5 * sizeof(float));
		}
		unsigned int nStride = (3 + 2) * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(FaceMesh->nTriangle * 3, 0, 0);
	}

}

void CFaceEffect2DBlendBG::MergeVertex(float * pVertex, float * pUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		m_pMergeVertex = new float[nVertex*(3 + 2)];
	}
	for (int i = 0;i<nVertex;i++)
	{
		m_pMergeVertex[i * 5] = pVertex[i*3];
		m_pMergeVertex[i * 5 + 1] = pVertex[i * 3 + 1];
		m_pMergeVertex[i * 5 + 2] = pVertex[i * 3 + 2];
		m_pMergeVertex[i * 5 + 3] = pUV[i * 2];
		m_pMergeVertex[i * 5 + 4] = pUV[i * 2 + 1];
	}
}

void CFaceEffect2DBlendBG::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{

		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
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
				}

			}

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
		}


		//anidrawable 
		nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
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

		nodeDrawable = childNode.getChildNode("videodrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					//AnimInfo anim_info;
					//anim_info.fps = 0;
					//anim_info.dir = path;
					//anim_info.relative_filename_list.push_back(szDrawableName);
					//m_anim_id = ResourceManager::Instance().loadAnim(anim_info);

					////m_drawable = new BitmapDrawable(pTex);
				}
				else
				{
					VideoInfo video_info;
					video_info.dir = path;
					video_info.relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadVideoFromZip(video_info, hZip);
				}

			}

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
		}


	}
}

bool CFaceEffect2DBlendBG::WriteConfig(std::string & tempPath, XMLNode & root, HZIP dst, HZIP src)
{

	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	std::string typeEffect = "FaceEffect2DWithBG";
	if (m_EffectPart == FACE_BLUSH)
	{
		typeEffect = "FaceBlush";
	}
	else if (m_EffectPart == FACE_BLEND_BLUSH)
	{
		typeEffect = "FaceBlendBlush";
	}
	nodeEffect.addAttribute("type", typeEffect.c_str());
	nodeEffect.addAttribute("showname", m_showname.c_str());

	XMLNode nodeDrwable = nodeEffect.addChild("anidrawable");
	nodeDrwable.addAttribute("name", "11");

	GetBlendParm(m_BlendType);
	nodeDrwable.addAttribute("blendType", m_BlendName.c_str());
	char alpha[256];
	sprintf(alpha, "%.4f",m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);
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
