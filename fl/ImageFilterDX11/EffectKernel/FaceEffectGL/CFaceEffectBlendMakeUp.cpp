#include "CFaceEffectBlendMakeUp.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Drawable.hpp"
#include <iostream>


CFaceEffectBlendMakeUp::CFaceEffectBlendMakeUp()
{
	m_FaceMeshManage = NULL;
	m_pMergeVertex = NULL;
	m_isCrop = false;
}


CFaceEffectBlendMakeUp::~CFaceEffectBlendMakeUp()
{
	Release();
}

void CFaceEffectBlendMakeUp::Release()
{
	m_pShader.reset();
	SAFE_DELETE(m_FaceMeshManage);
	SAFE_DELETE_ARRAY(m_pMergeVertex);
	ReleaseMakeUpInfo();
}

void CFaceEffectBlendMakeUp::ReleaseMakeUpInfo()
{
	
	//for (int i = 0; i < m_vMakeUpBlendInfo.size(); i++)
	//{
	//	SAFE_DELETE(m_vMakeUpBlendInfo[i].m_material);
	//}
	m_vMakeUpBlendInfo.clear();
}

void * CFaceEffectBlendMakeUp::Clone()
{
	CFaceEffectBlendMakeUp* result = new CFaceEffectBlendMakeUp();
	*result = *this;
	return result;
}

bool CFaceEffectBlendMakeUp::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		int i = -1;
		ReleaseMakeUpInfo();
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++i);
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			MakeUpBlendInfo mInfo;
			if (szDrawableName != NULL)
			{
				//MaterialTex *pTex = new MaterialTex();
				//pTex->CreateTexFromZIP(hZip, szDrawableName);
				std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
				mInfo.m_drawable = std::make_shared<BitmapDrawable>(TexRHI);;
			}
			const char *szType = nodeDrawable.getAttribute("FaceType");
			const char *isCrop = nodeDrawable.getAttribute("Crop");
			if (isCrop != NULL && !strcmp(isCrop, "true"))
			{
				m_isCrop = true;
			}
			const char *szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				mInfo.alpha = atof(szAlpha);
			}
			if (szType != NULL && !strcmp(szType, "Eyebrow"))
			{
				mInfo.m_meshType = MESH_EYEBROW;
			}
			else if (szType != NULL && !strcmp(szType, "Eye"))
			{
				mInfo.m_meshType = MESH_EYE;
			}
			else if (szType != NULL && !strcmp(szType, "EyeMult"))
			{
				mInfo.m_meshType = MESH_EYE_LID;
			}
			else if (szType != NULL && !strcmp(szType, "Mouth"))
			{
				mInfo.m_meshType = MESH_MOUTH;
			}

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			mInfo.m_blendType = GetBlendType(szBlendType);
			m_vMakeUpBlendInfo.push_back(mInfo);

			nodeDrawable = childNode.getChildNode("drawable", ++i);
		}

		//anidrawable image
		nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			MakeUpBlendInfo mInfo;
			std::shared_ptr<AnimationDrawable> drawable = std::make_shared<AnimationDrawable>();
			const char *szDrawableName = nodeDrawable.getAttribute("name");
			const char *szOffset = nodeDrawable.getAttribute("offset");
			if (szOffset != NULL)
			{
				long offset = atol(szOffset);
				drawable->setOffset(offset);
			}
			const char *szType = nodeDrawable.getAttribute("FaceType");
			const char *isCrop = nodeDrawable.getAttribute("Crop");
			if (isCrop != NULL && !strcmp(isCrop, "true"))
			{
				m_isCrop = true;
			}
			const char *szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				mInfo.alpha = atof(szAlpha);
			}
			if (szType != NULL && !strcmp(szType, "Eyebrow"))
			{
				mInfo.m_meshType = MESH_EYEBROW;
			}
			else if (szType != NULL && !strcmp(szType, "Eye"))
			{
				mInfo.m_meshType = MESH_EYE;
			}
			else if (szType != NULL && !strcmp(szType, "EyeMult"))
			{
				mInfo.m_meshType = MESH_EYE_LID;
			}
			else if (szType != NULL && !strcmp(szType, "Mouth"))
			{
				mInfo.m_meshType = MESH_MOUTH;
			}

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
					for (; iStart <= nCount; iStart += nStep)
					{
						sprintf(szImagePath, szItems, iStart);

						sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);
						//MaterialTex *pTex = NULL;
						//if (pTex == NULL)
						//{
						//	pTex = new MaterialTex();
						//	pTex->CreateTexFromZIP(hZip, szImagePath);
						//}
						std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath);
						long during = nDuring;

						drawable->appandTex(during, TexRHI);
					}
				}
			}
			mInfo.m_drawable = drawable;
			m_vMakeUpBlendInfo.push_back(mInfo);
		}
		return true;
	}
	return false;
}



bool CFaceEffectBlendMakeUp::Prepare()
{
	m_pShader = std::make_shared<class CCProgram>();
	std::string  vspath = m_resourcePath + "/Shader/faceEffectBlendMakeUp.vs";
	std::string  fspath = m_resourcePath + "/Shader/faceEffectBlendMakeUp.fs";
	m_pShader->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

	m_FaceMeshManage = new FaceMeshManage();
	m_FaceMeshManage->m_isCrop = m_isCrop;
	return true;
}



bool CFaceEffectBlendMakeUp::DrawTriLineToFBO(GLfloat* points, int count, unsigned short* indexs) {
	if (m_pShader == nullptr) 
	{
		LOGE("m_pShader::DrawTriLineToFBO: program is nullptr !");
		return false;
	}


	unsigned short *lines = new unsigned short[count * 6];
	int lineCnt = count;
	unsigned short* pSrc = indexs;
	unsigned short* pDst = lines;
	for (int i = 0; i < lineCnt; i++) {
		pDst[0] = pSrc[0];
		pDst[1] = pSrc[1];
		pDst[2] = pSrc[1];
		pDst[3] = pSrc[2];
		pDst[4] = pSrc[2];
		pDst[5] = pSrc[0];
		pDst += 6;
		pSrc += 3;
	}
	glDrawElements(GL_LINES, lineCnt * 6, GL_UNSIGNED_SHORT, lines);
	m_pShader->DisableVertexAttribPointer("a_position");
	SAFE_DELETE_ARRAY(lines);
	return true;
}


void CFaceEffectBlendMakeUp::Render(BaseRenderParam& RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}

	long runTime = RenderParam.runtime;
	if (runTime < 0)
	{
		return;
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	m_pShader->Use();
	glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
	int nFaceCount = RenderParam.GetFaceCount();
	for (int t = 0; t < m_vMakeUpBlendInfo.size(); t++)
	{
		if (m_vMakeUpBlendInfo[t].alpha < 0.01)
		{
			continue;
		}
		if (m_vMakeUpBlendInfo[t].m_meshType == MESH_EYE_LID && !m_IsEyelidVisable)
		{
			continue;
		}


		int blendType = (int)m_vMakeUpBlendInfo[t].m_blendType;

		m_pShader->SetUniform1i("blendtype", blendType);		
		m_pShader->SetUniform1f("alpha", m_vMakeUpBlendInfo[t].alpha);


		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_130);

			auto FaceMesh = m_FaceMeshManage->GetMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), m_vMakeUpBlendInfo[t].m_meshType);
			pDoubleBuffer->SyncAToBRegion((float*)FaceMesh->m_pVertices, FaceMesh->m_nVerts, 2, 1);

	        pDoubleBuffer->BindFBOA();

			//m_vMakeUpBlendInfo[t].m_drawable->getTex(runTime)->bind(0);
			//if (!glIsTexture(m_vMakeUpBlendInfo[t].m_drawable->getTex(runTime)->m_tex))
			//{
			//	continue;
			//}
			std::shared_ptr<MaterialTexRHI> TexRHI = m_vMakeUpBlendInfo[t].m_drawable->GetTex(runTime);
			if (!TexRHI)
			{
				continue;
			}
			TexRHI->Bind(0);

			m_pShader->SetUniform1i("inputImageTexture", 0);

			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());
			GetDynamicRHI()->SetPSShaderResource(1, pDoubleBuffer->GetFBOTextureB());
			m_pShader->SetUniform1i("inputImageTexture2", 1);
			
			m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, (float*)FaceMesh->m_pVertices);
			m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->m_pUV);
			glDrawElements(GL_TRIANGLES, FaceMesh->m_nTriangle * 3, GL_UNSIGNED_SHORT, FaceMesh->m_pTriangle);
			//DrawTriLineToFBO((float*)FaceMesh->m_pVertices, FaceMesh->m_nTriangle, FaceMesh->m_pTriangle);
		}
	}
}

void CFaceEffectBlendMakeUp::MergeVertex(float * pVertex, float * pUV, int nVertex)
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
