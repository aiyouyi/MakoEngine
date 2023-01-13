#include "CFaceBodyApartPBRModel.h"
#include "Toolbox/HeaderModelForCull.h"
#include "Toolbox/FSObject.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/Render/CC3DPbrRender.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11Resource.h"

CFaceBodyApartPBRModel::CFaceBodyApartPBRModel()
{
	m_EffectPart = FACE_BODY_APART_3D_MODEL;
}

void CFaceBodyApartPBRModel::Release()
{
}

CFaceBodyApartPBRModel::~CFaceBodyApartPBRModel()
{
	Release();
}

void * CFaceBodyApartPBRModel::Clone()
{
	CFaceBodyApartPBRModel* result = new CFaceBodyApartPBRModel();
	*result = *this;
	return result;
}

bool CFaceBodyApartPBRModel::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
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
			const char* rotateNode = nodeGltfModel.getAttribute("rotateNodeName");
			if (rotateNode)
			{
				rotateNodeName = rotateNode;
			}
		}

		begin = std::chrono::system_clock::now();

		return false;
	}
}

bool CFaceBodyApartPBRModel::Prepare()
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

void CFaceBodyApartPBRModel::Render(BaseRenderParam & RenderParam)
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
	m_RenderUtils->RunTime = runTime;
	//frame = 0;
	//frame++;
	//if (frame>125)
	//{
	//	nface = 1;
	//}
	//else
	//{
	//	view project;

	//}

	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin);
	if (runTime < (6.23333 * 1000) )
	{
		//固定位置
		bFollowFace = false;
	}
	else
	{
		bFollowFace = true;
	}

	if (nFaceCount > 0)
	{
		m_RenderUtils->PreRender(RenderParam);
		//view matrix
		glm::mat4 matView = glm::lookAtLH(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f));

		auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

		if (m_RenderUtils->GetDoubleBuffer() != NULL)
		{
			pDoubleBuffer = m_RenderUtils->GetDoubleBuffer();
		}


		//渲染人头像信息,用于后续剔除(禁止混合，禁止写入颜色buffer，开启深度测试和深度buffer写)

		RHIResourceCast(pDoubleBuffer.get())->GetFBOB()->clear(0, 0, 0, 1);
		RHIResourceCast(pDoubleBuffer.get())->BindDoubleBuffer();
		RHIResourceCast(pDoubleBuffer.get())->GetFBOA()->clearDepth();
		for (int faceIndex = 0; faceIndex < nFaceCount; ++faceIndex)
		{
			GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
			GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable, 0);
			//构建人脸变换矩阵信息
			FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(faceIndex);


			glm::mat4 matRotateXYZ;
			memcpy((float *)(&matRotateXYZ[0][0]), pFaceInfo->arrWMatrix, sizeof(float) * 16);
			matRotateXYZ = glm::rotate(matRotateXYZ, glm::radians(m_rotateX), glm::vec3(1.0f, 0.f, 0.f));

			glm::mat4 matScale2 = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScaleHead));

			//Perspective matrix
			float fAngle = atan(0.5) * 2;
			if (pFaceInfo->pFaceRect.height > pFaceInfo->pFaceRect.width)
			{
				fAngle = atan(0.5*pFaceInfo->pFaceRect.height / pFaceInfo->pFaceRect.width) * 2;
			}

			glm::mat4 matScale3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
			glm::mat4 matProjDX = matScale3 * glm::perspectiveRH(fAngle, pFaceInfo->pFaceRect.width*1.f / pFaceInfo->pFaceRect.height, 10.0f, 2000.f);
			glm::mat4 matProjDXForHead = matScale3 * glm::perspectiveLH(fAngle, pFaceInfo->pFaceRect.width*1.f / pFaceInfo->pFaceRect.height, 10.0f, 2000.f);


			glm::mat4 matWVP = matProjDXForHead * matView * matRotateXYZ*matScale2;

			glm::mat4 matScale = glm::scale(glm::mat4(1.0f), glm::vec3(m_MatScale));

			//m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = matRotateXYZ * matScale;

			//if (m_RenderUtils->m_DyBone_array.size() > 0&&m_RenderUtils->m_3DScene->m_Skeleton.size() > 0 && m_RenderUtils->m_3DScene->m_Skeleton[0]->m_RootNode.size() > 0 && m_RenderUtils->m_3DScene->m_Model[0]->m_hasSkin)
			//{
			//	if (faceIndex == 0)
			//	{
			//		glm::mat4 matTranslate;
			//		matTranslate = glm::rotate(matTranslate, -pFaceInfo->pitch * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
			//		matTranslate = glm::rotate(matTranslate, -pFaceInfo->yaw * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
			//		matTranslate = glm::rotate(matTranslate, pFaceInfo->roll * CC_PI / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));

			//		m_RenderUtils->m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat = matTranslate;
			//		m_RenderUtils->m_3DScene->m_Skeleton[0]->UpdateBone();
			//	}
			//	m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix * glm::inverse(m_RenderUtils->m_3DScene->m_Skeleton[0]->m_RootNode[0]->ParentMat);
			//}

			glm::mat4 translation;
			translation = glm::translate(translation, glm::vec3(pFaceInfo->x, pFaceInfo->y, -pFaceInfo->z));
			m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = translation * matScale;
			glm::mat4 matRotation;
			matRotation = glm::rotate(matRotation, -pFaceInfo->yaw * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));//绕头顶旋转
			matRotation = glm::rotate(matRotation, pFaceInfo->roll * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));//正确
			matRotation = glm::rotate(matRotation, pFaceInfo->pitch * CC_PI / 180.f, glm::vec3(0.0f, 0.0f, 1.0f));//点头抬头

			auto boneMap = m_RenderUtils->m_3DScene->m_Skeleton[0]->m_BoneMap;
			auto boneNodeArray = m_RenderUtils->m_3DScene->m_Skeleton[0]->m_BoneNode;
			auto boneItr = boneMap.find(rotateNodeName);
			if (boneItr != boneMap.end())
			{
				CC3DBoneNodeInfo* boneNodeInfo = boneNodeArray[boneItr->second];

				glm::quat quaternion = glm::quat_cast(matRotation);
				boneNodeInfo->TargetRotation = Vector4(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
				m_RenderUtils->m_3DScene->m_Skeleton[0]->UpdateBone();
			}


			m_RenderUtils->m_3DScene->m_Camera.m_ViewMatrix = glm::mat4(1.0);
			m_RenderUtils->m_3DScene->m_Project.m_ProjectMatrix = matProjDX;
			float blendFactor[] = { 0.f,0.f,0.f,0.f };
			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BSDisableWriteDisable, blendFactor, 0xffffffff);


			int faceL = pFaceInfo->pFaceRect.x;
			int faceT = pFaceInfo->pFaceRect.y;
			int faceW = pFaceInfo->pFaceRect.width;
			int faceH = pFaceInfo->pFaceRect.height;
			faceL -= 0.5*faceW;
			faceT -= 0.5*faceH;
			faceW *= 2;
			faceH *= 2;

			GetDynamicRHI()->SetViewPort(faceL, faceT, faceW, faceH);
			//绘制人头标准模型
			m_pShader->useShader();
			//SetParameter("matWVP", &matWVP, 0, sizeof(glm::mat4));
			GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).SetMatrix4Parameter("matWVP", &matWVP[0][0], false, 1);
			GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).ApplyToAllBuffer();
			GetDynamicRHI()->DrawPrimitive(VerticeBuffer, IndexBuffer);

			GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

			if (!bMatrixInit)
			{
				bMatrixInit = true;
				worldMat = m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix;
				projMat = m_RenderUtils->m_3DScene->m_Project.m_ProjectMatrix;
				//m_pre_position = { pFaceInfo->x, pFaceInfo->y, -pFaceInfo->z };
				facerect = { 0, 0, width, height };
			}

			//不跟随人脸
			if (!bFollowFace)
			{
				m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = worldMat;
				m_RenderUtils->m_3DScene->m_Project.m_ProjectMatrix = projMat;
				GetDynamicRHI()->SetViewPort(facerect.x, facerect.y, facerect.z, facerect.w);
				//GetDynamicRHI()->SetViewPort(0, 0, width, height);
				m_RenderUtils->m_3DScene->m_nSelectCamera = 1;
			}
			else
			{
				//开始跟随人脸，需要修改m_ModelMatrix;
				//glm::vec3 step_pos = glm::vec3(pFaceInfo->x, pFaceInfo->y, -pFaceInfo->z) - m_pre_position;
				glm::ivec4 step_client = glm::ivec4(faceL, faceT, faceW, faceH) - facerect;
				if (m_transition_frames > 0)
				{
					//step_pos.x /= m_transition_frames;
					//step_pos.y /= m_transition_frames;
					//step_pos.z /= m_transition_frames;

					//glm::vec3 new_pos = m_pre_position + step_pos;
					//glm::mat4 new_mat;
					//new_mat = glm::translate(new_mat, m_pre_position);
					//m_RenderUtils->m_3DScene->m_ModelControl.m_ModelMatrix = new_mat * matScale;
					////m_pre_position = new_pos;
					//printf("%d,%d,%d\n", facerect.x, facerect.y, facerect.z);

					step_client.x /= m_transition_frames;
					step_client.y /= m_transition_frames;
					step_client.z /= m_transition_frames;
					step_client.w /= m_transition_frames;

					glm::ivec4 new_client = facerect + step_client;
					facerect = new_client;
					GetDynamicRHI()->SetViewPort(new_client.x, new_client.y, new_client.z, new_client.w);
				}
				m_transition_frames--;
				m_RenderUtils->m_3DScene->m_nSelectCamera = -1;
			}

			m_RenderUtils->Render(RenderParam);

		}
		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);


	}
	m_RenderUtils->PostRender(RenderParam);



}
