#include "CFaceMakeUp.h"
#include "Toolbox\DXUtils\DXUtils.h"
#include <iostream>
#include "Toolbox\fileSystem.h"
#include "EffectKernel/ShaderProgramManager.h"

CFaceMakeUp::CFaceMakeUp()
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


CFaceMakeUp::~CFaceMakeUp()
{
	Release();
}

void CFaceMakeUp::Release()
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

void * CFaceMakeUp::Clone()
{
	CFaceMakeUp* result = new CFaceMakeUp();
	*result = *this;
	return result;
}

bool CFaceMakeUp::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFaceMakeUp::ReadConfig(XMLNode& childNode, const std::string &path)
{
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CFaceMakeUp::WriteConfig(XMLNode &root, HZIP dst, HZIP src)
{
// 	static int order = 0;
// 	XMLNode child = root.addChild("typeeffect");
// 	child.addAttribute("type", "FaceMakeUp");
// 
// 	std::string drawable_name = "FaceMakeUp" + std::to_string(order++);
// 	string img_folder = getTempFolder() + "/" + drawable_name;
// 	_mkdir(img_folder.c_str());
// 
// 	const std::string face_types[] = { "Eyebrow", "Eye", "Mouth" };
// 	for (int i = 0; i < m_vMeshType.size(); i++) {
// 		XMLNode drawable = child.addChild("drawable");
// 		string face_type = face_types[m_vMeshType[i].m_meshType];
// 		drawable.addAttribute("FaceType", face_type.c_str());
// 		string filename = face_type + "_" + std::to_string(i) + ".png";
// 		drawable.addAttribute("Material", (drawable_name + "/" + filename).c_str());
// 		{
// 			int w = m_vMeshType[i].m_material->width();
// 			int h = m_vMeshType[i].m_material->height();
// 			unsigned char *buf = new unsigned char[w * h * 4];
// 			m_vMeshType[i].m_material->ReadTextureToCpu(buf);
// 			cv::Mat img(h, w, CV_8UC4, buf);
// 			cv::cvtColor(img, img, CV_BGRA2RGBA);
// 			string img_name = img_folder + "/" + filename;
// 			cv::imwrite(img_name, img);
// 			delete buf;
// 		}
// 	}

	return true;
}

bool CFaceMakeUp::Prepare()
{
	//´´½¨shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	if (m_EffectPart == FACE_EYESTRICK)
	{
		for (int i = 0; i < m_vMeshType.size(); i++)
		{
			m_vMeshType[i].alpha = 0.0;
		}

	}

	string path = m_resourcePath + "/Shader/face2dMakeUp.fx";
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

void CFaceMakeUp::Render(BaseRenderParam &RenderParam)
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

	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);
	
	for (int t = 0;t<m_vMeshType.size();t++)
	{
		float pParam[4];
		pParam[0] = m_vMeshType[t].alpha;
		if (pParam[0]<0.01)
		{
			continue;
		}
		
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		//auto pMaterialView = m_vMeshType[t].m_drawable->GetSRV(runTime);
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
		auto pMaterialView = m_vMeshType[t].m_drawable->GetTex(runTime);
		pMaterialView->Bind(0);

		int nFaceCount = RenderParam.GetFaceCount();
		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_130);
			auto FaceMesh = m_FaceMeshManage->GetMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), m_vMeshType[t].m_meshType);
			MergeVertex((float*)FaceMesh->m_pVertices, (float*)FaceMesh->m_pUV, FaceMesh->m_nVerts);


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

CFaceMakeUp * CFaceMakeUp::createEffect()
{
	CFaceMakeUp* effect = new CFaceMakeUp();
	char *path = "./BuiltInResource/createEffect/FaceMakeUP.zip";
	CEffectPart::createEffect(path, "FaceMakeUp", effect);
	return effect;
}

void CFaceMakeUp::ReleaseMakeUpInfo()
{
	for (int i = 0; i < m_vMeshType.size(); i++)
	{
		SAFE_DELETE(m_vMeshType[i].m_drawable);
	}
	m_vMeshType.clear();
}

void CFaceMakeUp::MergeVertex(float * pVertex, float * pUV, int nVertex)
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

void CFaceMakeUp::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		int i = -1;
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++i);
		ReleaseMakeUpInfo();
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			MakeUpInfo mInfo;

			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					std::string szFullFile = path + "/" + szDrawableName;
					auto TexRHI = GetDynamicRHI()->CreateAsynTextureFromFile(szFullFile);
					mInfo.m_drawable = new BitmapDrawable(TexRHI);
				}
				else
				{
					auto TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName, false);
					mInfo.m_drawable = new BitmapDrawable(TexRHI);
				}

			}

			const char *szType = nodeDrawable.getAttribute("FaceType");

			const char *isCrop = nodeDrawable.getAttribute("Crop");
			if (isCrop != NULL && !strcmp(isCrop, "true"))
			{
				m_isCrop = true;
			}

			FaceMeshType meshType;


			if (szType != NULL && !strcmp(szType, "Eyebrow"))
			{
				mInfo.m_meshType = MESH_EYEBROW;//;
			}
			else if (szType != NULL && !strcmp(szType, "Mouth"))
			{
				mInfo.m_meshType = MESH_MOUTH;//;
			}
			else if (szType != NULL && !strcmp(szType, "Eye"))
			{
				mInfo.m_meshType = MESH_EYE;//;
			}

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
			MakeUpInfo mInfo;

			AnimationDrawable *drawable = NULL;
			if (drawable == NULL)
			{
				drawable = new AnimationDrawable();
			}

			const char *szType = nodeDrawable.getAttribute("FaceType");
			const char *isCrop = nodeDrawable.getAttribute("Crop");
			if (isCrop != NULL && !strcmp(isCrop, "true"))
			{
				m_isCrop = true;
			}
			else
			{
				m_isCrop = true;
			}
			if (szType != NULL && !strcmp(szType, "Eyebrow"))
			{
				mInfo.m_meshType = MESH_EYEBROW;//;
			}
			else if (szType != NULL && !strcmp(szType, "Mouth"))
			{
				mInfo.m_meshType = MESH_MOUTH;//;
			}
			else if (szType != NULL && !strcmp(szType, "Eye"))
			{
				mInfo.m_meshType = MESH_EYE;//;
			}

			const char *szDelayTime = nodeDrawable.getAttribute("DelayTime");
			if (szDelayTime != NULL)
			{
				//delayTime = atol(szDelayTime);
			}

			const char *szOffset = nodeDrawable.getAttribute("offset");
			long offset = atol(szOffset);
			drawable->setOffset(offset);

			const char *szLoopMode = nodeDrawable.getAttribute("loopMode");
			if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
			{
				drawable->setLoopMode(ELM_ONESHOT);
			}
			else
			{
				drawable->setLoopMode(ELM_REPEAT);
			}

			const char *szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
			{
				bGenMipmap = true;
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
				if (nCount > 0 && nDuring > 0)
				{
					char szFullFile[256];
					for (; iStart <= nCount; iStart += nStep)
					{
						sprintf(szImagePath, szItems, iStart);

						if (hZip == 0)
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);

							std::shared_ptr< MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureFromFile(szFullFile);

							long during = nDuring;

							drawable->appandTex(during, TexRHI);
						}
						else 
						{
							sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);
							std::shared_ptr< MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath, bGenMipmap);

							long during = nDuring;

							drawable->appandTex(during, TexRHI);

						}

					}
				}
			}

			mInfo.m_drawable = drawable;
			m_vMeshType.push_back(mInfo);
			nodeDrawable = childNode.getChildNode("anidrawable", ++i);
		}
	}
}