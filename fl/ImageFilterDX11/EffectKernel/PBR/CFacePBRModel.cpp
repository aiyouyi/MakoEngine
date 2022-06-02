#include "CFacePBRModel.h"
#include "Toolbox/HeaderModelForCull.h"
#include "Toolbox/FSObject.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/Render/CC3DPbrRender.h"
#include "Toolbox/RenderState/PiplelineState.h"

CFacePBRModel::CFacePBRModel()
{
	m_EffectPart = FACE_PBR_3D_MODEL;
}

void CFacePBRModel::Release()
{
}

CFacePBRModel::~CFacePBRModel()
{
	Release();
}

void* CFacePBRModel::Clone()
{
	CFacePBRModel* result = new CFacePBRModel();
	*result = *this;
	return result;
}

bool CFacePBRModel::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath)
{

	if (!childNode.isEmpty())
	{
		if (!m_RenderUtils)
		{
			m_RenderUtils = std::make_shared<CRenderPBRModel>();
			m_RenderUtils->ReadConfig(childNode, hZip, pFilePath);
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

			const char* szGradient = nodeTime.getAttribute("Gradient");
			if (szGradient != NULL)
			{
				sscanf(szGradient, "%f,%f,%f", m_GradientTime, m_GradientTime + 1, m_GradientTime + 2);
			}

		}


		//model
		XMLNode nodeGltfModel = childNode.getChildNode("gltf", 0);
		if (!nodeGltfModel.isEmpty())
		{

			const char* matScale = nodeGltfModel.getAttribute("matScale");
			if (matScale != NULL)
			{
				sscanf(matScale, "%f", &m_MatScale);
			}

			const char* matHead = nodeGltfModel.getAttribute("matScaleHead");
			if (matHead != NULL)
			{
				sscanf(matHead, "%f", &m_MatScaleHead);
			}
			const char* rotateX = nodeGltfModel.getAttribute("rotateX");
			if (rotateX != NULL)
			{
				sscanf(rotateX, "%f", &m_rotateX);
			}
		}



		return false;
	}
}

bool CFacePBRModel::Prepare()
{
	VerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(g_headerModelForCullVertices, g_headerModelForCullVerticesCount, 3);
	IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(g_headerModelForCullIndces, g_headerModelForCullFaceCount);

	// 头模shader
	//m_pShader = new DX11Shader();
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
	};
	std::string path = m_resourcePath + "/Shader/face3dForCullFace.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1, true);

	if (m_RenderUtils)
	{
		m_RenderUtils->Prepare(m_resourcePath);
	}

	return true;
}

void CFacePBRModel::Render(BaseRenderParam& RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	if (!m_RenderUtils)
	{
		return;
	}
	if (m_RenderUtils->m_3DScene == NULL)
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
	int nFaceCount = RenderParam.GetFaceCount();

	float alpha = 1.0;
	if (m_GradientTime[0] > 0 && runTime < m_GradientTime[0])
	{
		alpha = runTime / m_GradientTime[0];
	}
	if (m_GradientTime[2] > m_GradientTime[1] && runTime > m_GradientTime[1])
	{
		alpha = (m_GradientTime[2] - runTime) / (m_GradientTime[2] - m_GradientTime[1]);
		if (alpha < 0)
		{
			alpha = 0;
		}
	}
	m_RenderUtils->m_runTime = runTime;
	if (nFaceCount > 0)
	{
		m_RenderUtils->PreRender(RenderParam);
		//view matrix
		glm::mat4 matView = glm::lookAtLH(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));

		auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

		if (m_RenderUtils->m_DoubleBuffer != NULL)
		{
			pDoubleBuffer = m_RenderUtils->m_DoubleBuffer;
		}


		//渲染人头像信息,用于后续剔除(禁止混合，禁止写入颜色buffer，开启深度测试和深度buffer写)

		pDoubleBuffer->GetFBOB()->clear(0, 0, 0, 1);
		pDoubleBuffer->BindDoubleBuffer();
		pDoubleBuffer->GetFBOA()->clearDepth();
		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
			GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable, 0);
			//构建人脸变换矩阵信息
			FacePosInfo* pFaceInfo = RenderParam.GetFaceInfo(faceIndex);


			glm::mat4 matRotateXYZ;
			memcpy((float*)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);
			matRotateXYZ = glm::rotate(matRotateXYZ, glm::radians(m_rotateX), glm::vec3(1.0f, 0.f, 0.f));

			glm::mat4 matScale2 = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScaleHead));

			//Perspective matrix
			float fAngle = atan(0.5) * 2;
			if (pFaceInfo->pFaceRect.height > pFaceInfo->pFaceRect.width)
			{
				fAngle = atan(0.5 * pFaceInfo->pFaceRect.height / pFaceInfo->pFaceRect.width) * 2;
			}

			glm::mat4 matScale3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			glm::mat4 matProjDX = matScale3 * glm::perspectiveLH(fAngle, pFaceInfo->pFaceRect.width * 1.f / pFaceInfo->pFaceRect.height, 10.0f, 2000.f);


			glm::mat4 matWVP = matProjDX * matView * matRotateXYZ * matScale2;

			glm::mat4 matScale = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScale));

			m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = matRotateXYZ * matScale;

			if (m_RenderUtils->m_DyBone_array.size() > 0 && m_RenderUtils->m_3DScene->m_Skeleton.size() > 0 && m_RenderUtils->m_3DScene->m_Skeleton[0]->m_RootNode.size() > 0 && m_RenderUtils->m_3DScene->m_Model[0]->m_hasSkin)
			{
				if (faceIndex == 0)
				{
					glm::mat4 matTranslate;
					matTranslate = glm::rotate(matTranslate, -pFaceInfo->pitch * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
					matTranslate = glm::rotate(matTranslate, -pFaceInfo->yaw * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
					matTranslate = glm::rotate(matTranslate, pFaceInfo->roll * CC_PI / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));

					m_RenderUtils->m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat = matTranslate;
					m_RenderUtils->m_3DScene->m_Skeleton[0]->UpdateBone();
				}
				m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix * glm::inverse(m_RenderUtils->m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat);
			}

			m_RenderUtils->m_3DScene->m_Camera.m_ViewMatrix = matView;
			m_RenderUtils->m_3DScene->m_Project.m_ProjectMatrix = matProjDX;



			float blendFactor[] = { 0.f,0.f,0.f,0.f };
			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BSDisableWriteDisable, blendFactor, 0xffffffff);


			int faceL = pFaceInfo->pFaceRect.x;
			int faceT = pFaceInfo->pFaceRect.y;
			int faceW = pFaceInfo->pFaceRect.width;
			int faceH = pFaceInfo->pFaceRect.height;
			faceL -= 0.5 * faceW;
			faceT -= 0.5 * faceH;
			faceW *= 2;
			faceH *= 2;

			GetDynamicRHI()->SetViewPort(faceL, faceT, faceW, faceH);
			//绘制人头标准模型
			m_pShader->useShader();
			SetParameter("matWVP", &matWVP, 0, sizeof(glm::mat4));
			GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).ApplyToAllBuffer();
			GetDynamicRHI()->DrawPrimitive(VerticeBuffer, IndexBuffer);

			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);


			m_RenderUtils->Render(RenderParam);

		}
		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);


	}
	m_RenderUtils->PostRender(RenderParam);


}
