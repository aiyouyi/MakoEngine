#include "CFaceEffectMakeUp.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Toolbox/Render/DynamicRHI.h"


CFaceEffectMakeUp::CFaceEffectMakeUp()
{
	m_FaceMeshManage = NULL;
	m_pMergeVertex = NULL;
	m_isCrop = false;
}


CFaceEffectMakeUp::~CFaceEffectMakeUp()
{
	Release();
}

void CFaceEffectMakeUp::Release()
{
	m_pShader.reset();
	SAFE_DELETE(m_FaceMeshManage);
	SAFE_DELETE_ARRAY(m_pMergeVertex);
	//SAFE_RELEASE_BUFFER(m_pBlendStateMulity);
	ReleaseMakeUpInfo();
}

void * CFaceEffectMakeUp::Clone()
{
	CFaceEffectMakeUp* result = new CFaceEffectMakeUp();
	*result = *this;
	return result;
}

bool CFaceEffectMakeUp::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		int i = -1;
		ReleaseMakeUpInfo();
		XMLNode nodeDrawable = childNode.getChildNode("drawable", ++i);
		while (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			MakeUpInfo mInfo;
			if (szDrawableName != NULL)
			{
				mInfo.m_material = GetDynamicRHI()->CreateTextureFromZip(hZip, szDrawableName);
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
				mInfo.m_blendType = NORMAL_TYPE;
			}
			else if (szType != NULL && !strcmp(szType, "Eye"))
			{
				mInfo.m_meshType = MESH_EYE;//;
				mInfo.m_blendType = NORMAL_TYPE;
			}
			else if (szType != NULL && !strcmp(szType, "Mouth"))
			{
				mInfo.m_meshType = MESH_MOUTH;//;
				mInfo.m_blendType = MULTIPLY_TYPE;
			}
			m_vMakeUpBlendInfo.push_back(mInfo);

			nodeDrawable = childNode.getChildNode("drawable", ++i);

		}
		return true;
	}
	return false;
}

bool CFaceEffectMakeUp::Prepare()
{
	m_pShader = std::make_shared<class CCProgram>();
	std::string  vspath = m_resourcePath + "/Shader/faceEffectMakeUp.vs";
	std::string  fspath = m_resourcePath + "/Shader/faceEffectMakeUp.fs";
	m_pShader->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

	m_FaceMeshManage = new FaceMeshManage();
	m_FaceMeshManage->m_isCrop = m_isCrop;
	return true;
}


bool CFaceEffectMakeUp::DrawTriLineToFBO(GLfloat* points, int count, unsigned short* indexs) {
	if (m_pShader == nullptr) 
	{
		LOGE("m_pShader::DrawTriLineToFBO: program is nullptr !");
		return false;
	}
	//m_pShader->Use();
	//glViewport(0, 0, m_nCanvasWidth, m_nCanvasHeight);
	//mtlab::Matrix4 Projection = mtlab::setOrthoFrustum(0.0f, m_nCanvasWidth + 0.0f, 0.0f, m_nCanvasHeight + 0.0f, -1.0f, 1.0f);
	
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(0.0f, m_nCanvasWidth + 0.0f, 0.0f, m_nCanvasHeight + 0.0f, -1.0f, 1.0f);
/*	glm::mat4 Projection = glm::mat4(1.0f);
	Projection = glm::perspective(glm::radians(45.0f), (GLfloat)m_nCanvasWidth / (GLfloat)m_nCanvasHeight, 0.1f, 100.0f);


	GLfloat m_fBlue = 0.1f, m_fGreen = 0.1f, m_fRed = 0.3f;
	m_pShader->SetUniform3f("drawColor", m_fRed, m_fGreen, m_fBlue);
	
	m_pShader->SetUniformMatrix4fv("mvpMatrix", glm::value_ptr(Projection), false, 1);*///Projection.getTranspose()
	//m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, points);

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


void CFaceEffectMakeUp::Render(BaseRenderParam &RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();
	glColorMask(1, 1, 1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	m_pShader->Use();
	glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
	

	int nFaceCount = RenderParam.GetFaceCount();
	for (int t = 0; t < m_vMakeUpBlendInfo.size(); t++)
	{
		if (m_vMakeUpBlendInfo[t].m_blendType == NORMAL_TYPE)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (m_vMakeUpBlendInfo[t].m_blendType == MULTIPLY_TYPE)
		{
			glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		}
       // m_vMakeUpBlendInfo[t].m_material->bind(0);
		GetDynamicRHI()->SetPSShaderResource(0, m_vMakeUpBlendInfo[t].m_material);
		m_pShader->SetUniform1i("inputImageTexture", 0);
	
		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
			auto FaceMesh = m_FaceMeshManage->GetMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), m_vMakeUpBlendInfo[t].m_meshType);
			//MergeVertex((float*)FaceMesh->m_pVertices, (float*)FaceMesh->m_pUV, FaceMesh->m_nVerts);

			m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, (float*)FaceMesh->m_pVertices);
			m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->m_pUV);
            glDrawElements(GL_TRIANGLES, FaceMesh->m_nTriangle * 3, GL_UNSIGNED_SHORT, FaceMesh->m_pTriangle);
            
			//DrawTriLineToFBO((float*)FaceMesh->m_pVertices, FaceMesh->m_nTriangle, FaceMesh->m_pTriangle);

		}


	}
	glColorMask(1, 1, 1, 1);
	glDisable(GL_BLEND);
}


void CFaceEffectMakeUp::ReleaseMakeUpInfo()
{
	for (int i = 0; i < m_vMakeUpBlendInfo.size(); i++)
	{
		m_vMakeUpBlendInfo[i].m_material.reset();
	}
	m_vMakeUpBlendInfo.clear();
}

void CFaceEffectMakeUp::MergeVertex(float * pVertex, float * pUV, int nVertex)
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
