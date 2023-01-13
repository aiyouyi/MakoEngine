
#include "CC3DPbrRender.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include <algorithm>
#include "Effect/CC3DShadowMapManger.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "CC3DShadowRender.h"
#include "Material/CC3DMaterial.h"


namespace CC3DImageFilter
{

	CC3DPbrRender::CC3DPbrRender(void)
	{
		m_ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		m_nRenderWidth = 0;
		m_nRenderHeight = 0;
		m_pShaderBack = nullptr;
		m_3DSceneManage = nullptr;
		m_RenderCube = nullptr;

		m_ShadowMapMgr = std::make_shared<ShadowMapManager>();
		m_ShadowMapMgr->setShadowCascades(1);
	}

	CC3DPbrRender::~CC3DPbrRender(void)
	{
		SAFE_DELETE(m_RenderCube);
		SAFE_DELETE(m_pShaderBack);
	}


	void CC3DPbrRender::SetSize(uint32 nWidth, uint32 nHeight)
	{

		if (m_nRenderWidth != nWidth || m_nRenderHeight != nHeight)
		{
			m_nRenderWidth = nWidth;
			m_nRenderHeight = nHeight;
		}

	}

	void CC3DPbrRender::SetMatrix(CC3DMaterial* pMaterial)
	{
		if (m_3DSceneManage->m_nSelectCamera > 0 && m_3DSceneManage->m_Model[0]->m_CameraNode.m_CameraNode.size() > 0)
		{
			auto& ModelMat = m_3DSceneManage->m_Model[0]->m_CameraNode.m_CameraNode[m_3DSceneManage->m_nSelectCamera - 1]->FinalMeshMat;
			glm::vec4 vPos(0.0, 0.0, 0.0, 1.0);
			vPos = ModelMat * vPos;
			vPos /= vPos.w;

			glm::vec4 vDir(0.0, 0.0, -1.0, 1.0);
			vDir = ModelMat * vDir;
			vDir /= vDir.w;
			auto view = glm::lookAt(glm::vec3(vPos.x, vPos.y, vPos.z), glm::vec3(vDir.x, vDir.y, vDir.z), glm::vec3(0, 1, 0));
			auto Identity = glm::mat4();
			pMaterial->SetParameter("world", view);
			pMaterial->SetParameter("view", Identity);
			m_3DSceneManage->m_Model[0]->m_CameraNode.m_Project[m_3DSceneManage->m_nSelectCamera - 1].SetAspect(m_nRenderWidth * 1.0 / m_nRenderHeight);
			auto projection = m_3DSceneManage->m_Model[0]->m_CameraNode.m_Project[m_3DSceneManage->m_nSelectCamera - 1].GetProjectMatrix();
			pMaterial->SetParameter("projection", projection);
			pMaterial->SetParameter("camPos", Vector3(vPos.x, vPos.y, vPos.z));
		}
		else
		{
			auto camPos = m_3DSceneManage->m_Camera.GetCameraPos();
			pMaterial->SetParameter("camPos", Vector3(camPos.x, camPos.y, camPos.z));
			pMaterial->SetParameter("world", m_3DSceneManage->m_ModelControl.m_ModelMatrix);
			pMaterial->SetParameter("view", m_3DSceneManage->m_Camera.m_ViewMatrix);
			pMaterial->SetParameter("projection", m_3DSceneManage->m_Project.GetProjectMatrix());
		}

		glm::mat4 rotate = glm::rotate(glm::mat4(), EffectConfig->ModelConfig.hdrRotateX * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
		rotate = glm::rotate(rotate, EffectConfig->ModelConfig.hdrRotateY * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
		pMaterial->SetParameter("RotateIBL", rotate);

		if (EffectConfig->ModelConfig.bEnableShadow)
		{
			pMaterial->SetParameter("lightSpaceMatrix", m_ShadowRender->GetLightSpaceMatrix());
		}
	}

	void CC3DPbrRender::SetLight(CC3DMaterial* pMaterial)
	{
		int LightNum = m_3DSceneManage->m_Light.size();
		pMaterial->SetParameter("lightNum", LightNum);

		Vector4 lightDir[LIGHT_NUM];
		Vector4 lightColors[LIGHT_NUM];
		for (int i = 0; i < LightNum; i++)
		{
			CC3DDirectionalLight* pLight = (CC3DDirectionalLight*)m_3DSceneManage->m_Light[i];
			lightDir[i] = Vector4(pLight->m_LightDirection.x, pLight->m_LightDirection.y, pLight->m_LightDirection.z, pLight->m_LightIntensity);
			lightColors[i] = Vector4(pLight->m_LightColor.x, pLight->m_LightColor.y, pLight->m_LightColor.z, 1.0);
		}

		pMaterial->SetParameter("lightDir", lightDir);
		pMaterial->SetParameter("lightColors", lightColors);
	}

	void CC3DPbrRender::SetShaderResource(const std::string& path)
	{
		m_resourcePath = path;

		m_pDepthStateEnableWriteDisable = GetDynamicRHI()->CreateDefaultStencilState(true, false);
		m_pDepthStateEnableWriteEnable = GetDynamicRHI()->CreateDefaultStencilState(true, false);


		if (m_RenderCube == NULL)
		{
			m_RenderCube = new CC3DRenderCube();
		}
		m_RenderCube->SetShaderResource(path);

		if (!m_ShadowRender)
		{
			m_ShadowRender = std::make_shared<CC3DShadowRender>();
		}
		m_ShadowRender->SetShaderResource(path);
	}

	void CC3DPbrRender::SetClearColor(float r, float g, float b, float a)
	{
		m_ClearColor.r = ((std::max)(0.0f, ((std::min)(1.0f, r))));
		m_ClearColor.g = ((std::max)(0.0f, ((std::min)(1.0f, g))));
		m_ClearColor.b = ((std::max)(0.0f, ((std::min)(1.0f, b))));
		m_ClearColor.a = ((std::max)(0.0f, ((std::min)(1.0f, a))));
	}

	void CC3DPbrRender::Render(uint32 nWidth, uint32 nHeight)
	{
		SetSize(nWidth, nHeight);
		SetCommonUniform();

		PreRenderJob();
		RenderJob();

	}

	void CC3DPbrRender::RenderCube()
	{
		if (m_NeedRenderCube)
		{
			m_NeedRenderCube = false;
			m_RenderCube->Render();
		}
	}

	void CC3DPbrRender::SetHDRPath(const std::string& fileName)
	{
		if (m_RenderCube == NULL)
		{
			m_RenderCube = new CC3DRenderCube();
		}
		m_RenderCube->setHDR(fileName);
		m_NeedRenderCube = true;
	}

	void CC3DPbrRender::SetCommonUniform()
	{
		int nModel = m_3DSceneManage->m_Model.size();
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			int nMaterial = pModel->m_ModelMaterial.size();

			for (int j = 0; j < nMaterial; j++)
			{
				auto pMaterial = pModel->m_ModelMaterial[j];
				pMaterial->UseShader();
				SetMatrix(pMaterial);
				SetLight(pMaterial);
			}
		}
	}


	void CC3DPbrRender::SetAnimation(CC3DMesh* pMesh, int nSkeleton)
	{
		int skinID = pMesh->m_nSkinID;
		auto& Bone = m_3DSceneManage->m_Skeleton[nSkeleton]->m_BoneNodeArray[skinID];

		for (uint32 k = 0; k < Bone.size(); k++)
		{
			pMesh->m_Material->SetBoneMatrix(Bone[k].FinalMat, k);
		}

		if (m_ShadowRender)
		{
			m_ShadowRender->SetAnimation(*m_3DSceneManage, pMesh, nSkeleton);
		}
	}

	void CC3DPbrRender::PreRenderJob()
	{
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);

		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

		int nModel = m_3DSceneManage->m_Model.size();
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			int nMesh = pModel->m_ModelMesh.size();

			for (int j = 0; j < nMesh; j++)
			{
				CC3DMesh* pMesh = pModel->m_ModelMesh[j];

				if (!pMesh->m_isTransparent)
				{
					pMesh->m_Material->UseShader();
					pMesh->m_Material->SetParameter("bTransparent", 0);
					bool withAnimation = false;
					if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[i]->m_BoneNodeArray.size())
					{
						withAnimation = true;
						SetAnimation(pMesh, i);
					}
					PreRenderMesh(pMesh, withAnimation);
				}
			}
		}

		GetSortMeshID();
		int nMesh = m_SortMesh.size();

		for (int j = 0; j < nMesh; j++)
		{
			int type = m_SortMesh[j].PosType;
			int index = m_SortMesh[j].MeshID;
			int nModel = m_SortMesh[j].ModelID;
			CC3DModel* pModel = m_3DSceneManage->m_Model[nModel];
			CC3DMesh* pMesh = pModel->m_ModelMesh[index];
			if (pMesh->m_isTransparent)
			{
				pMesh->m_Material->UseShader();
				pMesh->m_Material->SetParameter("bTransparent", 1);
				if (type == 1)
				{
					GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
				}
				else
				{
					GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);
				}
				bool withAnimation = false;
				if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[nModel]->m_BoneNodeArray.size())
				{
					withAnimation = true;
					SetAnimation(pMesh, nModel);
				}
				PreRenderMesh(pMesh, withAnimation);
			}

		}
		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	}

	void CC3DPbrRender::PreRenderMesh(CC3DMesh* pMesh, bool withAnimation)
	{
		pMesh->m_Material->SetParameter("meshMat", pMesh->m_MeshMat);
		pMesh->m_Material->SetParameter("meshMatInverse", glm::inverse(pMesh->m_MeshMat));
		int enableAnimation = 0;
		if (withAnimation)
		{
			enableAnimation = 1;
		}
		pMesh->m_Material->SetParameter("AnimationEnable", enableAnimation);

		PreRenderSet(pMesh);
	}

	void CC3DPbrRender::PreRenderSet(CC3DMesh* pMesh)
	{
		pMesh->m_Material->SetTexture2D("irradianceMap", m_RenderCube->m_IrrCube);
		pMesh->m_Material->SetTexture2D("prefilterMap", m_RenderCube->m_PreCube);
		pMesh->m_Material->SetTexture2D("brdfLUT", m_RenderCube->m_BlutTexture);
		if (EffectConfig->ModelConfig.bEnableShadow)
		{
			pMesh->m_Material->SetTexture2D("ShadowMap", m_ShadowRender->GetShadowMap());
		}
		pMesh->m_Material->SetParameter("ShadowsEnable", EffectConfig->ModelConfig.bEnableShadow);
		pMesh->m_Material->PreRenderSet(pMesh);
	}

	void CC3DPbrRender::RenderJob()
	{

		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable, 0);
		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

		int nModel = m_3DSceneManage->m_Model.size();
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			int nMesh = pModel->m_ModelMesh.size();

			for (int j = 0; j < nMesh; j++)
			{
				CC3DMesh* pMesh = pModel->m_ModelMesh[j];

				if (!pMesh->m_isTransparent)
				{
					pMesh->m_Material->UseShader();

					pMesh->m_Material->GET_CONSTBUFFER(PBRConstantBuffer).EnableRenderOutLine = 0;
					bool withAnimation = false;
					if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[i]->m_BoneNodeArray.size())
					{
						withAnimation = true;
						SetAnimation(pMesh, i);
					}
					RenderMesh(pMesh, withAnimation);
				}
			}
		}

		if (EffectConfig->ModelConfig.bEnableOutLine)
		{
			RenderOutline();
		}

		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOff, blendFactor, 0xffffffff);
		if (!EffectConfig->ModelConfig.bEnableDepthWrite)
		{
			GetDynamicRHI()->SetDepthStencilState(m_pDepthStateEnableWriteDisable, 0);
		}

		int nMesh = m_SortMesh.size();

		for (int j = 0; j < nMesh; j++)
		{
			int type = m_SortMesh[j].PosType;
			int index = m_SortMesh[j].MeshID;
			int nModel = m_SortMesh[j].ModelID;
			CC3DModel* pModel = m_3DSceneManage->m_Model[nModel];
			CC3DMesh* pMesh = pModel->m_ModelMesh[index];
			if (pMesh->m_isTransparent)
			{
				pMesh->m_Material->UseShader();
				pMesh->m_Material->GET_CONSTBUFFER(PBRConstantBuffer).EnableRenderOutLine = 0;

				if (type == 1)
				{
					GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
				}
				else
				{
					//设置了毛发的材质为半透明，所以在渲染毛发背面时，应该跳过
					if (pMesh->m_Material->materialType == MaterialType::FUR)
					{
						continue;
					}
					GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);
				}
				bool withAnimation = false;
				if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[nModel]->m_BoneNodeArray.size())
				{
					withAnimation = true;
					SetAnimation(pMesh, nModel);
				}
				RenderMesh(pMesh, withAnimation);
			}
		}


		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);


	}

	void CC3DPbrRender::RenderMesh(CC3DMesh* pMesh, bool withAnimation)
	{
		pMesh->m_Material->SetParameter("meshMat", pMesh->m_MeshMat);
		pMesh->m_Material->SetParameter("meshMatInverse", glm::inverse(pMesh->m_MeshMat));
		int enableAnimation = 0;
		if (withAnimation)
		{
			enableAnimation = 1;
		}
		pMesh->m_Material->SetParameter("AnimationEnable", enableAnimation);

		RenderSet(pMesh);
	}

	void CC3DPbrRender::RenderSet(CC3DMesh* pMesh)
	{
		pMesh->m_Material->SetTexture2D("irradianceMap", m_RenderCube->m_IrrCube);
		pMesh->m_Material->SetTexture2D("prefilterMap", m_RenderCube->m_PreCube);
		pMesh->m_Material->SetTexture2D("brdfLUT", m_RenderCube->m_BlutTexture);

		if (EffectConfig->ModelConfig.bEnableShadow)
		{
			pMesh->m_Material->SetTexture2D("ShadowMap", m_ShadowRender->GetShadowMap());
		}
		pMesh->m_Material->SetParameter("ShadowsEnable", EffectConfig->ModelConfig.bEnableShadow);

		pMesh->m_Material->RenderSet(pMesh);

	}

	void CC3DPbrRender::RenderBackGround()
	{
		if (m_pShaderBack == NULL)
		{
			m_pShaderBack = new DX11Shader();

			CCVetexAttribute pAttribute[] =
			{
				{VERTEX_ATTRIB_POSITION, FLOAT_C3},
			};
			std::string  fspath = m_resourcePath + "/Shader/3D/background.fx";
			DXUtils::CompileShaderWithFile(m_pShaderBack, (char*)fspath.c_str(), pAttribute, 1);
			m_IBLCB = GetDynamicRHI()->CreateConstantBuffer(sizeof(IBLConstantBuffer));
		}
		m_pShaderBack->useShader();


		glm::mat4 view = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 rotate = glm::rotate(glm::mat4(), -EffectConfig->ModelConfig.hdrRotateX * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
		rotate = glm::rotate(rotate, -EffectConfig->ModelConfig.hdrRotateY * CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
		view *= rotate;

		glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)m_nRenderWidth / (float)m_nRenderHeight, 0.1f, 100.0f);

		m_IBLConstantBuffer.projection = glm::transpose(projection);
		m_IBLConstantBuffer.view = glm::transpose(view);

		GetDynamicRHI()->UpdateConstantBuffer(m_IBLCB, &m_IBLConstantBuffer);

		GetDynamicRHI()->SetPSShaderResource(0, m_RenderCube->m_IrrCube);
		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
		GetDynamicRHI()->SetVSConstantBuffer(0, m_IBLCB);
		GetDynamicRHI()->SetPSConstantBuffer(0, m_IBLCB);

		m_RenderCube->renderCube();
	}

	void CC3DPbrRender::SetModeConfig(CC3DImageFilter::EffectConfig* Config)
	{
		EffectConfig = Config;
	}

	void CC3DPbrRender::GetSortMeshID()
	{
		int nModel = m_3DSceneManage->m_Model.size();

		int nSumMesh = 0;
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			nSumMesh += pModel->m_ModelMesh.size();

		}
		if (nSumMesh > 0)
		{
			if (m_SortMesh.size() != nSumMesh * 2)
			{
				m_SortMesh.resize(nSumMesh * 2);
			}

		}
		CC3DImageFilter::MeshDisInfo* pSortMesh = &m_SortMesh[0];
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			int nMesh = pModel->m_ModelMesh.size();

			CC3DImageFilter::MeshDisInfo DisInfo;

			glm::mat4 mModelMatrix = m_3DSceneManage->m_ModelControl.GetModelMatrix();
			glm::vec3 mCameraPos = m_3DSceneManage->m_Camera.GetCameraPos();
			Vector3 vCamPos = Vector3(mCameraPos.x, mCameraPos.y, mCameraPos.z);
			DisInfo.ModelID = i;
			for (int j = 0; j < nMesh; j++)
			{
				DisInfo.MeshID = j;
				CC3DMesh* pMesh = pModel->m_ModelMesh[j];

				Vector3 minPoint = pMesh->m_meshBox.minPoint;
				Vector3 maxPoint = pMesh->m_meshBox.maxPoint;
				GetBoxPoint(minPoint, maxPoint);
				float distanceMin = 100000.f;
				float distanceMax = 0.f;

				for (int i = 0; i < m_BoxPoint.size(); i++)
				{
					Vector3 Point = m_BoxPoint[i];

					glm::vec4 mPoint = glm::vec4(Point[0], Point[1], Point[2], 1.0);
					glm::vec4 mTargetPoint = glm::vec4(mModelMatrix * pMesh->m_MeshMat * mPoint);
					mTargetPoint = mTargetPoint / mTargetPoint.w;

					//计算两个向量Z的距离
					Vector3 vTargetPoint = Vector3(mTargetPoint.x, mTargetPoint.y, mTargetPoint.z);
					float Distance = abs(vTargetPoint.z - vCamPos.z);
					//float Distance = vTargetPoint.distance(vCamPos);
					if (Distance < distanceMin)
					{
						distanceMin = Distance;

						DisInfo.PosType = 1;
						DisInfo.Distance = distanceMin;
						pSortMesh[j * 2] = DisInfo;
					}
					if (Distance > distanceMax)
					{
						distanceMax = Distance;

						DisInfo.PosType = 0;
						DisInfo.Distance = distanceMax;
						pSortMesh[j * 2 + 1] = DisInfo;
					}
				}
			}

			pSortMesh += nMesh * 2;

		}
		std::sort(m_SortMesh.begin(), m_SortMesh.end(), CC3DImageFilter::MeshDisInfo());
	}

	void CC3DPbrRender::GetBoxPoint(Vector3& minPoint, Vector3& maxPoint)
	{
		if (m_BoxPoint.size() != 8)
		{
			m_BoxPoint.resize(8);
		}
		m_BoxPoint[0] = minPoint;
		m_BoxPoint[1] = maxPoint;

		m_BoxPoint[2] = Vector3(minPoint.x, maxPoint.y, maxPoint.z);
		m_BoxPoint[3] = Vector3(minPoint.x, minPoint.y, maxPoint.z);
		m_BoxPoint[4] = Vector3(minPoint.x, maxPoint.y, minPoint.z);

		m_BoxPoint[5] = Vector3(maxPoint.x, minPoint.y, minPoint.z);
		m_BoxPoint[6] = Vector3(maxPoint.x, maxPoint.y, minPoint.z);
		m_BoxPoint[7] = Vector3(maxPoint.x, minPoint.y, maxPoint.z);
	}

	void CC3DPbrRender::RenderOutline()
	{
		if (!m_OutlineDepthState)
		{
			m_OutlineDepthState = GetDynamicRHI()->CreateDepthStencilState();

			m_OutlineDepthState->m_bDepthEnable = true;
			m_OutlineDepthState->m_uiDepthCompareMethod = CC3DDepthStencilState::CM_LESSEQUAL;
			m_OutlineDepthState->m_bDepthWritable = true;
			m_OutlineDepthState->m_bStencilEnable = false;

			m_OutlineDepthState->CreateState();
		}

		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		GetDynamicRHI()->SetDepthStencilState(m_OutlineDepthState);
		GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);

		int nModel = m_3DSceneManage->m_Model.size();
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			int nMesh = pModel->m_ModelMesh.size();

			for (int j = 0; j < nMesh; j++)
			{
				CC3DMesh* pMesh = pModel->m_ModelMesh[j];

				if (!pMesh->m_isTransparent)
				{
					std::string MaterialName = pMesh->m_Material->MaterialName;
					pMesh->m_Material->UseShader();
					pMesh->m_Material->GET_CONSTBUFFER(PBRConstantBuffer).EnableRenderOutLine = 1;

					if (MaterialName.find("Body") != std::string::npos)
					{
						pMesh->m_Material->SetTexture2D("OutLineMask", EffectConfig->HairOutLineMaskTex);
					}
					else
					{
						pMesh->m_Material->SetTexture2D("OutLineMask", EffectConfig->DefOutLineMaskTex);
					}

					bool withAnimation = false;
					if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[i]->m_BoneNodeArray.size())
					{
						withAnimation = true;
						SetAnimation(pMesh, i);
					}
					RenderMesh(pMesh, withAnimation);
				}
			}
		}
	}

	void CC3DPbrRender::RenderShadowMap()
	{
		if (!m_ShadowRender || !EffectConfig->ModelConfig.bEnableShadow)
			return;

		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);

		m_ShadowMapMgr->update(*m_3DSceneManage);
		//TODO:写死model，后续需要修改
		m_ShadowRender->Bind(*m_3DSceneManage, *m_ShadowMapMgr);

		int nModel = m_3DSceneManage->m_Model.size();
		for (int i = 0; i < nModel; i++)
		{
			CC3DModel* pModel = m_3DSceneManage->m_Model[i];
			int nMesh = pModel->m_ModelMesh.size();
			//GetSortMeshID();
			for (int j = 0; j < nMesh; j++)
			{
				//int type = m_SortMesh[j].PosType;
				//int index = m_SortMesh[j].MeshID;
				int index = j;
				CC3DMesh* pMesh = pModel->m_ModelMesh[index];
				bool withAnimation = false;
				if (pMesh->m_isTransparent)
				{
					continue;
				}
				if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[i]->m_BoneNodeArray.size())
				{
					withAnimation = true;
					m_ShadowRender->SetAnimation(*m_3DSceneManage, pMesh, i);
				}

				m_ShadowRender->RenderMesh(*pMesh, withAnimation);
			}
		}

		m_ShadowRender->UnBind();
		m_ShadowRender->ProcessBlur();

	}

}
