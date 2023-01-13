#include "CFacePBRModelGL.h"
#include "Toolbox/HeaderModelForCull.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/Render/CC3DPbrRender.h"
#include "CC3DEngine/RenderGL/CRenderPBRModelGL.h"
#include "BaseDefine/commonFunc.h"
#include "RenderState/PiplelineState.h"
#include "Toolbox/Drawable.hpp"
#include "Toolbox/GL/CCProgram.h"

CFacePBRModelGL::CFacePBRModelGL()
{
	m_EffectPart = FACE_PBR_3D_MODEL;
	m_pShaderForHeaderCull = nullptr;
	m_RenderUtil = nullptr;
}

CFacePBRModelGL::~CFacePBRModelGL()
{
	Release();
}

void CFacePBRModelGL::Release()
{
	m_RenderUtil.reset();
}

void * CFacePBRModelGL::Clone()
{
	CFacePBRModelGL* result = new CFacePBRModelGL();
	*result = *this;
	return result;
}

bool CFacePBRModelGL::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		if (!m_RenderUtil)
		{
			m_RenderUtil = std::make_shared<CRenderPBRModelGL>();
			m_RenderUtil->ReadConfig(childNode, hZip, pFilePath);
		}

		//model
		XMLNode nodeGltfModel = childNode.getChildNode("gltf", 0);
		if (!nodeGltfModel.isEmpty())
		{

			const char *matScale = nodeGltfModel.getAttribute("matScale");
			if (matScale != NULL)
			{
				sscanf(matScale, "%f", &m_MatScale);
			}

			const char *matHead = nodeGltfModel.getAttribute("matScaleHead");
			if (matHead != NULL)
			{
				sscanf(matHead, "%f", &m_MatScaleHead);
			}
			const char *rotateX = nodeGltfModel.getAttribute("rotateX");
			if (rotateX != NULL)
			{
				sscanf(rotateX, "%f", &m_rotateX);
			}
		}


	}
	return false;
}

bool CFacePBRModelGL::Prepare()
{
	VerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(g_headerModelForCullVertices, g_headerModelForCullVerticesCount, 3);
	IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(g_headerModelForCullIndces, g_headerModelForCullFaceCount);

	m_pShaderForHeaderCull = make_shared<CCProgram>();
	std::string vspath = m_resourcePath + "/Shader/HeadCullFace.vs";
	std::string fspath = m_resourcePath + "/Shader/HeadCullFace.fs";
	m_pShaderForHeaderCull->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
	GET_SHADER_STRUCT_MEMBER(HeadCull).Shader_ = m_pShaderForHeaderCull;

	if (m_RenderUtil == nullptr)
	{
		m_RenderUtil = make_shared<CRenderPBRModelGL>();
	}
	m_RenderUtil->Prepare(m_resourcePath);

	UpdateEmissMap();
	return true;
}

void CFacePBRModelGL::Render(BaseRenderParam & RenderParam)
{

	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount == 0)
	{
		return;
	}

	long runTime = GetRunTime();
	if (runTime < 0)
		return;

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	
	Vector4 faceOriRect = Vector4(0.0, 0.0, 1.0, 1.0);
	{	
		FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);
		
		if (pFaceInfo->pFaceRect.width > 0 && pFaceInfo->pFaceRect.height > 0)
		{
			faceOriRect.x = pFaceInfo->pFaceRect.x*1.f / width;
			faceOriRect.y = pFaceInfo->pFaceRect.y*1.f / height;
			faceOriRect.z = pFaceInfo->pFaceRect.width*1.f / width;
			faceOriRect.w = pFaceInfo->pFaceRect.height*1.f / height;
			width = pFaceInfo->pFaceRect.width;
			height = pFaceInfo->pFaceRect.height;
		}
	}

	//float fAngle = atan(0.5) * 2;
	//if (height > width)
	//{
	//	fAngle = atan(0.5*height / width) * 2;
	//}
    ////view matrix
	//glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));
	//glm::mat4 matProjDX = glm::perspective(fAngle, width*1.f / height, 10.0f, 2000.f);

	if (nFaceCount > 0)
	{
		//prepare render and src texture to HDR
		m_RenderUtil->PreRender(RenderParam);
	
		//GetDynamicRHI()->SetViewPort(0, 0, width, height);
		m_RenderUtil->BeginDrawPBRModelToFbo();
		m_RenderUtil->ClearDepth();
		GetDynamicRHI()->SetCullInverse(true); //Hack 写法，其他修改的需要注意
		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			auto pFaceInfo = RenderParam.GetFaceInfo(faceIndex);

			float fAngle = atan(0.5) * 2;
			if (0.5*pFaceInfo->pCameraRect.height > pFaceInfo->pCameraRect.width)
			{
				fAngle = atan(0.5*pFaceInfo->pCameraRect.height / pFaceInfo->pCameraRect.width) * 2;
			}
			glm::mat4 matView = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

			float nearZ = std::max(pFaceInfo->z - 100.f, 10.f);
			float farZ = std::min(2000.f, pFaceInfo->z + 100.f);

			glm::mat4 matProjDX = glm::perspective(fAngle, pFaceInfo->pCameraRect.width*1.f / pFaceInfo->pCameraRect.height, nearZ, farZ);
			
			glm::mat4 matScale3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			matProjDX = matScale3 * matProjDX;
		
			//matProjDX = matScale3 * matProjDX * matView;
			int faceL = pFaceInfo->pCameraRect.x;
			int faceT = pFaceInfo->pCameraRect.y;
			int faceW = pFaceInfo->pCameraRect.width;
			int faceH = pFaceInfo->pCameraRect.height;
			faceL -= 0.5*faceW;
			faceT -= 0.5*faceH;
			faceW *= 2;
			faceH *= 2;
			GetDynamicRHI()->SetViewPort(faceL, faceT, faceW, faceH);

			GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
			GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable, 0);
			//矩阵变换信息
			glm::mat4 matRotateXYZ = glm::mat4(1.0f);
		    memcpy((float *)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);
			matRotateXYZ = glm::rotate(matRotateXYZ, glm::radians(m_rotateX), glm::vec3(1.0f, 0.f, 0.f));
			glm::mat4 matScale2 = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScaleHead));
			
		    glm::mat4 matWVP =  matProjDX * matView* matRotateXYZ*matScale2;
					
			float blendFactor[] = { 0.f,0.f,0.f,0.f };
			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BSDisableWriteDisable, blendFactor, 0xffffffff);
			
			//绘制人头模型
			m_RenderUtil->DrawToColorBuffers(1);	
			m_pShaderForHeaderCull->Use();
			GetDynamicRHI()->SetColorMask(0, 0, 0, 0);
			//m_pShaderForHeaderCull->SetUniform4f("faceoriRect", faceOriRect.x, faceOriRect.y, faceOriRect.z, faceOriRect.w);
			//m_pShaderForHeaderCull->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, g_headerModelForCullVertices);
			//m_pShaderForHeaderCull->SetUniformMatrix4fv("u_matWVP", &matWVP[0][0], false, 1);
			//glDrawElements(GL_TRIANGLES, g_headerModelForCullFaceCount * 3, GL_UNSIGNED_SHORT, g_headerModelForCullIndces);
			
			GET_SHADER_STRUCT_MEMBER(HeadCull).SetMatrix4Parameter("u_matWVP", &matWVP[0][0], false, 1);
			GET_SHADER_STRUCT_MEMBER(HeadCull).SetParameter("faceoriRect", faceOriRect);
			GetDynamicRHI()->DrawPrimitive(VerticeBuffer, IndexBuffer);

			//绘制PBR模型
			GetDynamicRHI()->SetColorMask(1, 1, 1, 0);
	
			m_RenderUtil->DrawToColorBuffers(2);
			glm::mat4 matScale = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScale));
			m_RenderUtil->m_3DScene->m_ModelControl.m_ModelMatrix = matRotateXYZ * matScale;

			if (m_RenderUtil->CheckIfHasDynamicBone() > 0 && m_RenderUtil->m_3DScene->m_Skeleton.size() > 0 && m_RenderUtil->m_3DScene->m_Skeleton[0]->m_RootNode.size() > 0 && m_RenderUtil->m_3DScene->m_Model[0]->m_hasSkin)
			{
				if (faceIndex == 0)
				{
					glm::mat4 matTranslate;
					matTranslate = glm::rotate(matTranslate, -pFaceInfo->pitch * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
					matTranslate = glm::rotate(matTranslate, -pFaceInfo->yaw * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
					matTranslate = glm::rotate(matTranslate, pFaceInfo->roll * CC_PI / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));

					m_RenderUtil->m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat = matTranslate;
					m_RenderUtil->m_3DScene->m_Skeleton[0]->UpdateBone();
				}
				m_RenderUtil->m_3DScene->m_ModelControl.m_ModelMatrix = m_RenderUtil->m_3DScene->m_ModelControl.m_ModelMatrix * glm::inverse(m_RenderUtil->m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat);
			}

			m_RenderUtil->m_3DScene->m_Camera.m_ViewMatrix = matView;
			m_RenderUtil->m_3DScene->m_Project.m_ProjectMatrix = matProjDX;


			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
			m_RenderUtil->Render(RenderParam);
		}
		GetDynamicRHI()->SetCullInverse(false);
		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	}
	m_RenderUtil->PostRender(RenderParam);
}

void CFacePBRModelGL::UpdateEmissMap()
{

	//int nModel = m_RenderUtil->m_3DScene->m_Model.size();
	//for (int i = 0; i < nModel; i++)
	//{
	//	CC3DModel *pModel = m_RenderUtil->m_3DScene->m_Model[i];
	//	for (auto& mtl : pModel->m_ModelMaterialGL)
	//	{
	//		mtl->base_emiss_factor = m_EmissStength;
	//	}
	//}

}

