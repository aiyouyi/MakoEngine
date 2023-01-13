#include "CFaceBlendMakeUp.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include <iostream>
#include "Toolbox/fileSystem.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceBlendMakeUp::CFaceBlendMakeUp()
{

	m_FaceMeshManage = NULL;
	for (int t = 0; t < MAX_FACE_TYPE_NUM; t++)
	{
		for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
		{
			m_VerticeBuffer[t][i] = NULL;
		}
		m_IndexBuffer[t] = NULL;
	}
	m_pConstantBuffer = NULL;
	m_pMergeVertex = NULL;
	m_pBlendStateMulity = NULL;
	m_alpha = 1.0;
	m_isCrop = false;

}


CFaceBlendMakeUp::~CFaceBlendMakeUp()
{
	Release();
}

void CFaceBlendMakeUp::Release()
{
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_DELETE(m_FaceMeshManage);
	for (int t = 0; t < MAX_FACE_TYPE_NUM; t++)
	{
		for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
		{
			SAFE_RELEASE_BUFFER(m_VerticeBuffer[t][i]);
		}
		SAFE_RELEASE_BUFFER(m_IndexBuffer[t]);
	}
	SAFE_DELETE_ARRAY(m_pMergeVertex);
	SAFE_RELEASE_BUFFER(m_pBlendStateMulity);
	ReleaseMakeUpInfo();
}

void * CFaceBlendMakeUp::Clone()
{
	CFaceBlendMakeUp* result = new CFaceBlendMakeUp();
	*result = *this;
	return result;
}

bool CFaceBlendMakeUp::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFaceBlendMakeUp::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CFaceBlendMakeUp::Prepare()
{
	//创建shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};

	string path = m_resourcePath + "/Shader/face2dBlendMakeUp.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);
	m_FaceMeshManage = new FaceMeshManage();
	m_pBlendStateMulity = DXUtils::CreateBlendState(D3D11_BLEND_ZERO, D3D11_BLEND_SRC_COLOR);
	m_FaceMeshManage->m_isCrop = m_isCrop;

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	return true;
}

void CFaceBlendMakeUp::Render(BaseRenderParam &RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	//	DXUtils::SetRasterizerState(false);
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
//	pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();

	//float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	for (int t = 0; t < m_vMeshType.size(); t++)
	{
		float pParam[4];
		pParam[0] = m_vMeshType[t].alpha;
		if (pParam[0] < 0.01)
		{
			continue;
		}
		if (m_vMeshType[t].m_meshType == MESH_EYE_LID && !m_IsEyelidVisable)
		{
			continue;
		}

		pParam[1] = GetBlendParm(m_vMeshType[t].m_blendType);
		if (m_EnableMp4Alpha)pParam[2] = 1.0; else pParam[2] = 0.0;
		Image* img = ResourceManager::Instance().getAnimFrame(m_vMeshType[t].m_anim_id, float(runTime));
		auto pMaterialView = RHIResourceCast(img->tex.get())->GetSRV();
		//auto pMaterialView = m_vMeshType[t].m_drawable->getTex(runTime);
	
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
		
		int nFaceCount = RenderParam.GetFaceCount();
		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_130);
			auto FaceMesh = m_FaceMeshManage->GetMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), m_vMeshType[t].m_meshType);
			MergeVertex((float*)FaceMesh->m_pVertices, (float*)FaceMesh->m_pUV, FaceMesh->m_nVerts);

			pDoubleBuffer->SyncAToBRegion((float*)FaceMesh->m_pVertices, FaceMesh->m_nVerts,2,1);
			//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
			//DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
			GetDynamicRHI()->SetPSShaderResource(1, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());

			if (m_IndexBuffer[t] == NULL)
			{
				m_IndexBuffer[t] = DXUtils::CreateIndexBuffer(FaceMesh->m_pTriangle, FaceMesh->m_nTriangle);
			}
			if (m_VerticeBuffer[t][faceIndex] == NULL)
			{
				m_VerticeBuffer[t][faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, FaceMesh->m_nVerts, 5);
			}
			else
			{
				DXUtils::UpdateVertexBuffer(m_VerticeBuffer[t][faceIndex], m_pMergeVertex, FaceMesh->m_nVerts, 5 * sizeof(float), 5 * sizeof(float));
			}
			unsigned int nStride = (3 + 2) * sizeof(float);
			unsigned int nOffset = 0;


			DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[t][faceIndex], &nStride, &nOffset);
			DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer[t], DXGI_FORMAT_R16_UINT, 0);
			DeviceContextPtr->DrawIndexed(FaceMesh->m_nTriangle * 3, 0, 0);
		}

	}

}

void CFaceBlendMakeUp::SetAlpha(float Alpha)
{
	CEffectPart::SetAlpha(Alpha);
	for (size_t i = 0; i < m_vMeshType.size(); i++)
	{
		m_vMeshType[i].alpha = Alpha;
	}
}

CFaceBlendMakeUp * CFaceBlendMakeUp::createEffect()
{
	CFaceBlendMakeUp* effect = new CFaceBlendMakeUp();
	char *path = "./BuiltInResource/createEffect/FaceMakeUP.zip";
	CEffectPart::createEffect(path, "FaceMakeUp", effect);
	return effect;
}

void CFaceBlendMakeUp::ReleaseMakeUpInfo()
{
	for (int i = 0; i < m_vMeshType.size(); i++)
	{
		ResourceManager::Instance().freeAnim(m_vMeshType[i].m_anim_id);;
	}
	m_vMeshType.clear();
}

void CFaceBlendMakeUp::MergeVertex(float * pVertex, float * pUV, int nVertex)
{
	if (m_pMergeVertex == NULL)

	{
		int nSize = m_FaceMeshManage->GetMaxVerts();
		m_pMergeVertex = new float[nSize*(3 + 2)];
	}
	for (int i = 0; i < nVertex; i++)
	{
		m_pMergeVertex[i * 5] = pVertex[i * 2];
		m_pMergeVertex[i * 5 + 1] = pVertex[i * 2 + 1];
		m_pMergeVertex[i * 5 + 2] = 0.5;
		m_pMergeVertex[i * 5 + 3] = pUV[i * 2];
		m_pMergeVertex[i * 5 + 4] = pUV[i * 2 + 1];
	}

	// 	freopen("point.txt", "w", stdout);
	// 	std::cout << nVertex << std::endl;
	// 	for (int i = 0; i < nVertex; i++) {
	// 		std::cout << pVertex[i * 2]*0.5+0.5<< "," << pVertex[i * 2+1]*0.5+0.5 << ",";
	// 	}
	// 	fclose(stdout);
}

void CFaceBlendMakeUp::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		int i = -1;
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++i);
		ReleaseMakeUpInfo();
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			MakeUpBlendInfo mInfo;

			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{

					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					mInfo.m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
				}
				else
				{
					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					mInfo.m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
				}

			}

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha=mInfo.alpha = atof(szAlpha);
			}

			const char *szType = nodeDrawable.getAttribute("FaceType");
			mInfo.m_meshType = GetMeshType(szType);

			const char *isCrop = nodeDrawable.getAttribute("Crop");
			if (isCrop != NULL && !strcmp(isCrop, "true"))
			{
				m_isCrop = true;
			}
	
			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			mInfo.m_blendType = GetBlendType(szBlendType);

			m_vMeshType.push_back(mInfo);

			nodeDrawable = childNode.getChildNode("drawable", ++i);
		}
		//time
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
		//anidrawable 
		i = -1;
		nodeDrawable = childNode.getChildNode("anidrawable", ++i);
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");
			MakeUpBlendInfo mInfo;

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha=mInfo.alpha = atof(szAlpha);
			}

			const char *isCrop = nodeDrawable.getAttribute("Crop");
			if (isCrop != NULL && !strcmp(isCrop, "true"))
			{
				m_isCrop = true;
			}

			const char *szType = nodeDrawable.getAttribute("FaceType");
			mInfo.m_meshType = GetMeshType(szType);

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			mInfo.m_blendType = GetBlendType(szBlendType);

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
						mInfo.m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
					}
					else
					{
						mInfo.m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
					}
				}
			}

			m_vMeshType.push_back(mInfo);
			nodeDrawable = childNode.getChildNode("anidrawable", ++i);

		}
	}
}

FaceMeshType CFaceBlendMakeUp::GetMeshType(const char *szType)
{
	if (szType != NULL && !strcmp(szType, "Eyebrow"))
	{
		return MESH_EYEBROW;
	}
	else if (szType != NULL && !strcmp(szType, "Eye"))
	{
		return MESH_EYE;
	}
	else if (szType != NULL && !strcmp(szType, "EyeMult"))
	{
		return MESH_EYE_LID;
	}
	else
	{
		return MESH_MOUTH;
	}
}

bool CFaceBlendMakeUp::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "MakeUpBlend");
	nodeEffect.addAttribute("showname", m_showname.c_str());

	////时间控制
    //XMLNode time = nodeEffect.addChild("time");
	FileManager::Instance().SetSaveFolder(tempPath);
	for (int t = 0; t < m_vMeshType.size(); t++)
	{
		Anim* anim = ResourceManager::Instance().getAnim(m_vMeshType[t].m_anim_id);
		AnimInfo animInfo = anim->info;

		XMLNode nodeDrwable = nodeEffect.addChild("anidrawable");
		nodeDrwable.addAttribute("name", "11");

		char alpha[256];
		sprintf(alpha, "%.4f", m_vMeshType[t].alpha);
		nodeDrwable.addAttribute("alpha", alpha);

		if (m_vMeshType[t].m_meshType == MESH_EYEBROW)
		{
			nodeDrwable.addAttribute("FaceType", "Eyebrow");

			AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info, 0.25, 0.3, 0.75, 0.5);
			animInfo.relative_filename_list = renamed_info.relative_filename_list;

		}
		else if (m_vMeshType[t].m_meshType == MESH_EYE)
		{
			nodeDrwable.addAttribute("FaceType", "Eye");

			AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info, 0.25, 0.3, 0.75, 0.5);
			animInfo.relative_filename_list = renamed_info.relative_filename_list;
		}
		else if (m_vMeshType[t].m_meshType == MESH_MOUTH)
		{
			nodeDrwable.addAttribute("FaceType", "Mouth");

			AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info, 0.25, 0.5, 0.75, 0.7);
			animInfo.relative_filename_list = renamed_info.relative_filename_list;
		}
		else
		{
			nodeDrwable.addAttribute("FaceType", "EyeMult");

			AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info, 0.25, 0.3, 0.75, 0.5);
			animInfo.relative_filename_list = renamed_info.relative_filename_list;
		}

		nodeDrwable.addAttribute("Crop", "true");

		GetBlendParm(m_vMeshType[t].m_blendType);
		nodeDrwable.addAttribute("blendType", m_BlendName.c_str());
		
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
	return true;
}

void CFaceBlendMakeUp::setAnim(AnimInfo &info)
{
	if (m_vMeshType.size() == 1)
	{
		ResourceManager::Instance().freeAnim(m_vMeshType[0].m_anim_id);
		m_vMeshType[0].m_anim_id = ResourceManager::Instance().loadAnim(info);
		m_isCrop = ResourceManager::Instance().SetAnimCrop(m_vMeshType[0].m_anim_id);
		m_FaceMeshManage->m_isCrop = m_isCrop;
	}
}

void CFaceBlendMakeUp::getAnim(AnimInfo &info)
{
	if (m_vMeshType.size() == 1)
	{
		Anim* anim = ResourceManager::Instance().getAnim(m_vMeshType[0].m_anim_id);
		info = anim->info;
	}
}

void CFaceBlendMakeUp::setAnimFps(float fps)
{
	if (m_vMeshType.size() == 1)
	{
		ResourceManager::Instance().SetAnimFps(m_vMeshType[0].m_anim_id, fps);
	}
	
}