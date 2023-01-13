
#include "CC3DPbrRenderGL.h"
#include <algorithm>
#include "CC3DEngine/Render/CC3DShadowRender.h"
#include "Effect/CC3DShadowMapManger.h"
#include "CC3DPostProcessManagerGL.h"
#include "ToolBox/RenderState/PiplelineState.h"
#include "ToolBox/GL/GLResource.h"
#include "Common/CC3DEnvironmentConfig.h"


CC3DPbrRenderGL::CC3DPbrRenderGL(void)
{
	m_ClearColor = glm::vec4(0.0f,0.0f,0.0f,1.0f);
	m_nRenderWidth = 0;
	m_nRenderHeight = 0;
	m_MsaaFrameBuffer = NULL;
	m_FrameBuffer = NULL;

	m_3DSceneManage = NULL;
	m_RenderCube = NULL;

	_shadowMapManager = std::make_shared<CC3DImageFilter::ShadowMapManager>();
	_shadowMapManager->setShadowCascades(1);
}

CC3DPbrRenderGL::~CC3DPbrRenderGL(void)
{
	SAFE_DELETE(m_MsaaFrameBuffer);
	//SAFE_DELETE(m_FrameBuffer);
	SAFE_DELETE(m_RenderCube);
}


void CC3DPbrRenderGL::SetSize(uint32 nWidth, uint32 nHeight)
{
	if (m_nRenderHeight == 0)
	{
		m_MsaaFrameBuffer = new MSAAFramebuffer();
		m_MsaaFrameBuffer->Initialize();
	}
	if (nWidth <= 0 || nHeight <= 0)
	{
		return;
	}
	if (m_nRenderWidth != nWidth || m_nRenderHeight != nHeight)
	{
		m_nRenderWidth = nWidth;
		m_nRenderHeight = nHeight;
		//SAFE_DELETE(m_FrameBuffer);
		//m_FrameBuffer = new CCMRTFrameBuffer();
		//m_FrameBuffer->init(nWidth, nHeight);
		//m_MsaaFrameBuffer->AsFrameBuffer(m_FrameBuffer->framebufferID);
		//m_MsaaFrameBuffer->AsFrameBufferTexture(m_FrameBuffer->textureID);
		//m_MsaaFrameBuffer->AsEmissTexture(m_FrameBuffer->emissID);
		m_MsaaFrameBuffer->SetupSize(m_nRenderWidth, m_nRenderHeight);
	}


}

void CC3DPbrRenderGL::SetCommon()
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

void CC3DPbrRenderGL::SetMatrix(CC3DMaterial* mtl)
{
	if (m_3DSceneManage->m_nSelectCamera>0&&m_3DSceneManage->m_Model[0]->m_CameraNode.m_CameraNode.size() > 0)
	{
		auto &ModelMat = m_3DSceneManage->m_Model[0]->m_CameraNode.m_CameraNode[m_3DSceneManage->m_nSelectCamera-1]->FinalMeshMat;
		glm::vec4 vPos(0.0, 0.0, 0.0, 1.0);
		vPos = ModelMat * vPos;
		vPos /= vPos.w;

		glm::vec4 vDir(0.0, 0.0, -1.0, 1.0);
		vDir = ModelMat * vDir;
		vDir /= vDir.w;


		auto view = glm::lookAt(glm::vec3(vPos.x, vPos.y, vPos.z), glm::vec3(vDir.x, vDir.y, vDir.z), glm::vec3(0, 1, 0));
		auto Identity = glm::mat4();
		mtl->SetParameter("model", view); // 为什么设置反掉了
		mtl->SetParameter("view", Identity);

		m_3DSceneManage->m_Model[0]->m_CameraNode.m_Project[m_3DSceneManage->m_nSelectCamera - 1].SetAspect(m_nRenderWidth*1.0 / m_nRenderHeight);
		mtl->SetParameter("projection", m_3DSceneManage->m_Model[0]->m_CameraNode.m_Project[m_3DSceneManage->m_nSelectCamera-1].GetProjectMatrix());
		mtl->SetParameter("camPos", Vector3(0, 0, 0));
	}
	else
	{

		mtl->SetParameter( "world", m_3DSceneManage->m_ModelControl.m_ModelMatrix);
		mtl->SetParameter( "view", m_3DSceneManage->m_Camera.m_ViewMatrix);
		mtl->SetParameter( "projection", m_3DSceneManage->m_Project.GetProjectMatrix());

		glm::mat4 identy = glm::mat4(1.0f);

		glm::vec3& camPos = m_3DSceneManage->m_Camera.GetCameraPos();

		mtl->SetParameter("camPos", Vector3(camPos.x, camPos.y, camPos.z));
	}

	if (mtl->materialType == MaterialType::PBR )
	{
		glm::mat4 rotate = glm::rotate(glm::mat4(), Config->hdrRotateX*CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
		rotate = glm::rotate(rotate, Config->hdrRotateY*CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
		//mtl->SetUniformMatrix4fv("RotateIBL", &rotate[0][0], false, 1);
		mtl->SetParameter( "RotateIBL", rotate);

		if (Config->bEnableShadow)
		{
			//mtl->SetUniformMatrix4fv("lightSpaceMatrix", &_shadowRender->GetLightSpaceMatrix()[0][0], false, 1);
			mtl->SetParameter( "lightSpaceMatrix", _shadowRender->GetLightSpaceMatrix());
		}
	}
}

void CC3DPbrRenderGL::SetLight(CC3DMaterial* mtl)
{
	int LightNum = m_3DSceneManage->m_Light.size();
	//mtl->SetUniform1i("LightNum", LightNum);
	mtl->SetParameter( "LightNum", LightNum);

	Vector4 lightDir[LIGHT_NUM];
	Vector4 lightColors[LIGHT_NUM];
	for (int i = 0; i < LightNum; i++)
	{
		CC3DDirectionalLight* pLight = (CC3DDirectionalLight*)m_3DSceneManage->m_Light[i];
		lightDir[i] = Vector4(pLight->m_LightDirection.x, pLight->m_LightDirection.y, pLight->m_LightDirection.z, pLight->m_LightIntensity);
		lightColors[i] = Vector4(pLight->m_LightColor.x, pLight->m_LightColor.y, pLight->m_LightColor.z, 1.0);

	}

	mtl->SetParameter<Vector4, LIGHT_NUM>("lightDir", lightDir);
	mtl->SetParameter<Vector4, LIGHT_NUM>("lightColors", lightColors);

}

void CC3DPbrRenderGL::SetShaderResource(std::string  path)
{
	m_resourcePath = path;

	m_pShaderBack = std::make_shared<CCProgram>();
	std::string  vspath3 = m_resourcePath + "/Shader/3D/background.vs";
	std::string  fspath3 = m_resourcePath + "/Shader/3D/background.fs";
	m_pShaderBack->CreatePorgramForFile(vspath3.c_str(), fspath3.c_str());
	GET_SHADER_STRUCT_MEMBER(RenderBackGround).Shader_ = m_pShaderBack;



	if (m_RenderCube == NULL)
	{
		m_RenderCube = new CC3DRenderCubeGL();
	}
	m_RenderCube->SetShaderResource(path);
}

void CC3DPbrRenderGL::SetClearColor(float r, float g, float b, float a)
{
	m_ClearColor.r = ((std::max)(0.0f, ((std::min)(1.0f, r))));
	m_ClearColor.g = ((std::max)(0.0f, ((std::min)(1.0f, g))));
	m_ClearColor.b = ((std::max)(0.0f, ((std::min)(1.0f, b))));
	m_ClearColor.a = ((std::max)(0.0f, ((std::min)(1.0f, a))));
}

void CC3DPbrRenderGL::update()
{
	if (_shadowMapManager && Config->bEnableShadow)
	{
		_shadowMapManager->update(*m_3DSceneManage);
	}
}

void CC3DPbrRenderGL::Render(uint32 nWidth, uint32 nHeight)
{
	SetSize(nWidth, nHeight);

	m_MaterialType.clear();
    m_FrameBuffer->AttatchColorBuffers(2);

	SetCommon();
	PreRenderJob();
	RenderJob();
	GL_CHECK();

}

void CC3DPbrRenderGL::RenderCube()
{
	if (m_NeedRenderCube)
	{
		m_NeedRenderCube = false;
		m_RenderCube->Render();
	}
}

void CC3DPbrRenderGL::OutRenderShadowMap()
{
	if (Config->bEnableShadow)
	{
		if (_shadowRender == nullptr)
		{
			_shadowRender = make_shared<CC3DShadowRender>();
			_shadowRender->SetShaderResource(m_resourcePath);
		}

		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);

		_shadowRender->setModelMatrix(m_3DSceneManage->m_ModelControl.GetModelMatrix());
		RenderShadowMap();

		GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
		GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	}
}

void CC3DPbrRenderGL::SetHDRPath(std::string fileName)
{
	if (m_RenderCube == NULL)
	{
		m_RenderCube = new CC3DRenderCubeGL();
	}

	if (_shadowRender == nullptr)
	{
		_shadowRender = make_shared<CC3DShadowRender>();
	}

	m_RenderCube->setHDR(fileName);
	m_NeedRenderCube = true;
}

void CC3DPbrRenderGL::SetHDRData(float *pData, int &nWidth, int &nHeight)
{
	if (m_RenderCube == NULL)
	{
		m_RenderCube = new CC3DRenderCubeGL();
	}
	m_RenderCube->setHDRData(pData, nWidth, nHeight);
	m_NeedRenderCube = true;
}


void CC3DPbrRenderGL::SetAnimation(CC3DMesh* pMesh, int nSkeleton)
{
	int skinID = pMesh->m_nSkinID;
	auto &Bone = m_3DSceneManage->m_Skeleton[nSkeleton]->m_BoneNodeArray[skinID];

	for (uint32 k = 0; k < Bone.size(); k++)
	{
		pMesh->m_Material->SetBoneMatrix(Bone[k].FinalMat, k);
	}

}

void CC3DPbrRenderGL::RenderMesh(CC3DMesh* pMesh, bool withAnimation)
{

	int animationEnable = withAnimation ? 1 : 0;
	pMesh->m_Material->SetParameter( "meshMat", pMesh->m_MeshMat );
	pMesh->m_Material->SetParameter( "AnimationEnable", animationEnable);


	RenderSet(pMesh);
}

void CC3DPbrRenderGL::RenderShadowMap()
{
	if (_shadowRender == nullptr)
		return;
	
	_shadowMapManager->update(*m_3DSceneManage);
	//TODO:写死model，后续需要修改
	_shadowRender->Bind(*m_3DSceneManage, *_shadowMapManager);

	int nModel = m_3DSceneManage->m_Model.size();
	for (int i = 0; i < nModel; i++)
	{
		CC3DModel *pModel = m_3DSceneManage->m_Model[i];
		int nMesh = pModel->m_ModelMesh.size();
		//GetSortMeshID();
		if (nModel > 1&&i==0)
		{
			_shadowRender->ProjectShadow = 0.f;
		}
		else
		{
			_shadowRender->ProjectShadow = 1.f;
		}
		for (int j = 0; j < nMesh; j++)
		{
			//int type = m_SortMesh[j].PosType;
			//int index = m_SortMesh[j].MeshID;
			int index = j;
			CC3DMesh *pMesh = pModel->m_ModelMesh[index];
			bool withAnimation = false;
			if (pMesh->m_isTransparent)
			{
				continue;
			}
			if (pModel->m_hasSkin && pMesh->m_nSkinID >= 0 && pMesh->m_nSkinID < m_3DSceneManage->m_Skeleton[i]->m_BoneNodeArray.size())
			{
				withAnimation = true;
				_shadowRender->SetAnimation(*m_3DSceneManage, pMesh,i);
			}

			GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);
			GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);

			_shadowRender->RenderMesh(*pMesh, withAnimation);
		}
	}

	_shadowRender->UnBind();

	_shadowRender->ProcessBlur();
	

}

void CC3DPbrRenderGL::PreRenderJob()
{
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);


	int nModel = m_3DSceneManage->m_Model.size();
	for (int i = 0; i < nModel; i++)
	{
		CC3DModel *pModel = m_3DSceneManage->m_Model[i];
		int nMesh = pModel->m_ModelMesh.size();

		for (int j = 0; j < nMesh; j++)
		{
			CC3DMesh *pMesh = pModel->m_ModelMesh[j];

			if (!pMesh->m_isTransparent)
			{
				pMesh->m_Material->UseShader();
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
		CC3DModel *pModel = m_3DSceneManage->m_Model[nModel];
		CC3DMesh *pMesh = pModel->m_ModelMesh[index];
		if (pMesh->m_isTransparent)
		{
			pMesh->m_Material->UseShader();
			if (type == 0)
			{
				GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);
			}
			else
			{
				GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
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

	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
}

void CC3DPbrRenderGL::PreRenderMesh(CC3DMesh* pMesh, bool withAnimation)
{
	pMesh->m_Material->SetParameter("meshMat", pMesh->m_MeshMat);
	pMesh->m_Material->SetParameter("AnimationEnable", withAnimation);

	PreRenderSet(pMesh);
}

void CC3DPbrRenderGL::PreRenderSet(CC3DMesh* pMesh)
{
// 	SetMatrix(pMesh->m_Material);
// 	SetLight(pMesh->m_Material);
	pMesh->m_Material->SetTexture2D("irradianceMap", m_RenderCube->IrrCube);
	pMesh->m_Material->SetTexture2D("brdfLUT", m_RenderCube->Blut);
	pMesh->m_Material->SetTexture2D("prefilterMap", m_RenderCube->PreCube);

	if (Config->bEnableShadow)
	{
		pMesh->m_Material->SetTexture2D("shadowMap", _shadowRender->GetShadowMap());
	}

	//pMesh->m_MaterialGL->SetUniform1i("shadowsEnable", shadowMapEnable);
	pMesh->m_Material->SetParameter( "shadowsEnable", Config->bEnableShadow);

	pMesh->m_Material->PreRenderSet(pMesh); //材质系统
}

void CC3DPbrRenderGL::RenderJob()
{

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);
	GetDynamicRHI()->SetDepthWritableState(CC3DPiplelineState::DepthStateWriteEnable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);

	int nModel = m_3DSceneManage->m_Model.size();
	for (int i = 0; i < nModel; i++)
	{
		CC3DModel *pModel = m_3DSceneManage->m_Model[i];
		int nMesh = pModel->m_ModelMesh.size();

		for (int j = 0; j < nMesh; j++)
		{
			CC3DMesh *pMesh = pModel->m_ModelMesh[j];

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
				RenderMesh(pMesh, withAnimation);
			}
		}
	}


	if (!Config->bEnableDepthWrite)
	{
		GetDynamicRHI()->SetDepthWritableState(CC3DPiplelineState::DepthStateWriteDisable);
	}
	//	

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendAlphaOn, blendFactor, 0xffffffff);

	//GetSortMeshID();
	int nMesh = m_SortMesh.size();

	for (int j = 0; j < nMesh; j++)
	{
		int type = m_SortMesh[j].PosType;
		int index = m_SortMesh[j].MeshID;
		int nModel = m_SortMesh[j].ModelID;
		CC3DModel *pModel = m_3DSceneManage->m_Model[nModel];
		CC3DMesh *pMesh = pModel->m_ModelMesh[index];
		if (pMesh->m_isTransparent)
		{
			pMesh->m_Material->UseShader();
			pMesh->m_Material->SetParameter("bTransparent", 1);

			if (type == 0)
			{
				if (pMesh->m_Material->materialType == MaterialType::FUR )
				{
					continue;
				}

				GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullFront);
			}
			else
			{
				GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);
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

	GetDynamicRHI()->SetDepthWritableState(CC3DPiplelineState::DepthStateWriteEnable);
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);

}

void CC3DPbrRenderGL::RenderSet(CC3DMesh * pMesh)
{
// 	SetMatrix(pMesh->m_Material);
// 	SetLight(pMesh->m_Material);
	pMesh->m_Material->SetTexture2D("irradianceMap", m_RenderCube->IrrCube);
	pMesh->m_Material->SetTexture2D("brdfLUT", m_RenderCube->Blut);
	pMesh->m_Material->SetTexture2D("prefilterMap", m_RenderCube->PreCube);

	if (Config->bEnableShadow)
	{
		pMesh->m_Material->SetTexture2D("shadowMap", _shadowRender->GetShadowMap());
	}

	//pMesh->m_MaterialGL->SetUniform1i("shadowsEnable", shadowMapEnable);
	pMesh->m_Material->SetParameter("shadowsEnable", Config->bEnableShadow);

	pMesh->m_Material->RenderSet(pMesh); //材质系统
}


void CC3DPbrRenderGL::RenderBackGround()
{
	if (m_RenderCube->EnvCube == nullptr)
	{
		return;
	}
	m_pShaderBack->Use();
	glm::mat4 view = glm::lookAt(glm::vec3(0.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 rotate = glm::rotate(glm::mat4(), -Config->hdrRotateX * CC_PI / 180.f, glm::vec3(1.0f, 0.0f, 0.0f));
	rotate = glm::rotate(rotate, -Config->hdrRotateY*CC_PI / 180.f, glm::vec3(0.0f, 1.0f, 0.0f));
	view *= rotate;


	m_pShaderBack->SetUniformMatrix4fv("view", &view[0][0], false, 1);
	// 	glm::mat4 rotate = m_rotate;
	// 	//rotate = glm::rotate(rotate, 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	// 	m_programBG->SetUniformMatrix4fv("rotate", &rotate[0][0], false, 1);
	glm::mat4 projection = glm::perspective(glm::radians(90.f), (float)m_nRenderWidth / (float)m_nRenderHeight, 0.1f, 100.0f);

	GET_SHADER_STRUCT_MEMBER(RenderBackGround).SetMatrix4Parameter("projection", &projection[0][0], false, 1);
	GET_SHADER_STRUCT_MEMBER(RenderBackGround).SetTexture("environmentMap", m_RenderCube->IrrCube);

	m_RenderCube->renderCube();
}


uint32 CC3DPbrRenderGL::GetRenderResultTextureID()
{
	if (m_FrameBuffer!=NULL)
	{
		return m_FrameBuffer->textureID;
	}
	return 0;
}

void CC3DPbrRenderGL::ResetUVAnimation()
{
	_timeElapse_glass = 0.0f;
	_timeElapse_wave = 0.0f;
	_timeElapse_particle = 0.0f;
	uv_rotate_time = 0.0f;
}

void CC3DPbrRenderGL::SetModeConfig(CC3DImageFilter::ModelConfig* _ModelConfig)
{
	Config = _ModelConfig;
}

void CC3DPbrRenderGL::GetSortMeshID()
{
	int nModel = m_3DSceneManage->m_Model.size();

	int nSumMesh = 0;
	for (int i = 0; i < nModel; i++)
	{
		CC3DModel *pModel = m_3DSceneManage->m_Model[i];
		nSumMesh += pModel->m_ModelMesh.size();

	}
	if (nSumMesh>0)
	{
		if (m_SortMesh.size() != nSumMesh * 2)
		{
			m_SortMesh.resize(nSumMesh * 2);
		}

	}
	CC3DImageFilter::MeshDisInfo *pSortMesh = &m_SortMesh[0];
	for (int i = 0; i < nModel; i++)
	{
		CC3DModel *pModel = m_3DSceneManage->m_Model[i];
		int nMesh = pModel->m_ModelMesh.size();

		CC3DImageFilter::MeshDisInfo DisInfo;

		glm::mat4 mModelMatrix = m_3DSceneManage->m_ModelControl.GetModelMatrix();
		glm::vec3 mCameraPos = m_3DSceneManage->m_Camera.GetCameraPos();
		Vector3 vCamPos = Vector3(mCameraPos.x, mCameraPos.y, mCameraPos.z);
		DisInfo.ModelID = i;
		for (int j = 0; j < nMesh; j++)
		{
			DisInfo.MeshID = j;
			CC3DMesh *pMesh = pModel->m_ModelMesh[j];
			
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

void CC3DPbrRenderGL::GetBoxPoint(Vector3& minPoint, Vector3& maxPoint)
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
