#include "CC3DShadowRender.h"
#include "Model/CC3DMesh.h"
#include "Scene/CC3DSceneManage.h"
#include "Scene/CC3DCamera.h"
#include "Effect/CC3DShadowMap.h"
#include "Effect/CC3DShadowMapManger.h"
#include "CC3DBlurPass.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/DXUtils/DX11Resource.h"

CC3DShadowRender::CC3DShadowRender()
{
	mBlurPass = std::make_shared<CC3DBlurPass>();
	
}

CC3DShadowRender::~CC3DShadowRender()
{
	mBlurPass.reset();
	mDepthRenderBuffer.reset();
}

void CC3DShadowRender::Bind(CC3DSceneManage& scene_manager, ShadowMapManager& shadowManager)
{
	setModelMatrix(scene_manager.m_ModelControl.GetModelMatrix());

	CC3DDirectionalLight *pLight = (CC3DDirectionalLight*)scene_manager.m_Light[0];
	if (pLight != nullptr )
	{
		mLightDir = pLight->m_LightDirection;
		mLightDir = glm::normalize(mLightDir);
		mLightDir.z = -mLightDir.z;
	}
	

	mLightLookAt = glm::vec3(0.0f); //此处没有考虑相机平移的情况
	
	mLightUp = glm::vec3(0.0f, 1.0, 0.0f);
	mLightPos = mLightLookAt + (-mLightDir * LIGHT_DISTANCE);

	glm::vec3 z_axis = glm::normalize(mLightLookAt - mLightPos);
	if (abs(glm::dot( z_axis, mLightUp )) > 0.999)
	{
		mLightUp = { mLightUp.z, mLightUp.x, mLightUp.y };
	}

	glm::mat4 lightView = glm::lookAt(mLightPos, mLightLookAt, mLightUp);

	CC3DModel* render_model = scene_manager.m_Model[0];
	Vector3 minPoint = render_model->m_ModelBox.minPoint;
	Vector3 maxPoint = render_model->m_ModelBox.maxPoint;
	std::vector<Vector3>m_BoxPoint;
	m_BoxPoint.resize(8);
	for (int m=0;m< scene_manager.m_Model.size();m++)
	{
		render_model = scene_manager.m_Model[m];
		Vector3 minPoint1 = render_model->m_ModelBox.minPoint;
		Vector3 maxPoint1 = render_model->m_ModelBox.maxPoint;

		m_BoxPoint[0] = minPoint1;
		m_BoxPoint[1] = maxPoint1;
		m_BoxPoint[2] = Vector3(minPoint1.x, maxPoint1.y, maxPoint1.z);
		m_BoxPoint[3] = Vector3(minPoint1.x, minPoint1.y, maxPoint1.z);
		m_BoxPoint[4] = Vector3(minPoint1.x, maxPoint1.y, minPoint1.z);
		m_BoxPoint[5] = Vector3(maxPoint1.x, minPoint1.y, minPoint1.z);
		m_BoxPoint[6] = Vector3(maxPoint1.x, maxPoint1.y, minPoint1.z);
		m_BoxPoint[7] = Vector3(maxPoint1.x, minPoint1.y, maxPoint1.z);
		for (int i = 0; i < m_BoxPoint.size(); i++)
		{
			Vector3 Point = m_BoxPoint[i];

			glm::vec4 mPoint = glm::vec4(Point[0], Point[1], Point[2], 1.0);
			glm::vec4 mTargetPoint = glm::vec4(lightView * mModel_matrix * mPoint);
			mTargetPoint = mTargetPoint / mTargetPoint.w;

			//计算两个向量Z的距离
			Vector3 vTargetPoint = Vector3(mTargetPoint.x, mTargetPoint.y, mTargetPoint.z);
			if (i==0&&m==0)
			{
				minPoint = vTargetPoint;
				maxPoint = vTargetPoint;
				continue;
			}

			minPoint.x = (std::min)(minPoint.x, vTargetPoint.x);
			minPoint.y = (std::min)(minPoint.y, vTargetPoint.y);
			minPoint.z = (std::min)(minPoint.z, vTargetPoint.z);
										
			maxPoint.x = (std::max)(maxPoint.x, vTargetPoint.x);
			maxPoint.y = (std::max)(maxPoint.y, vTargetPoint.y);
			maxPoint.z = (std::max)(maxPoint.z, vTargetPoint.z);
		}
	}
// 
	float l = minPoint.x;
	float b = minPoint.y;
	float n = minPoint.z;
	float r = maxPoint.x;
	float t = maxPoint.y;
	float f = maxPoint.z;
// 	float offest = 0.2*(r - l);
// 	l = l - offest;
// 	r = r + offest;
// 	t = t + offest;
// 	b = b - offest;

	glm::vec4 spherecenter_ls = lightView * mModel_matrix * glm::vec4(mLightLookAt, 0.0f);
	spherecenter_ls.z = (mLightPos - mLightLookAt).length();


	glm::mat4 lightProjection;

	ShadowMap& shadow_map = *shadowManager.getShadowMap(0);
	mNearAndFar = glm::vec2(shadow_map.lsNear, shadow_map.lsFar);
	lightProjection = glm::ortho(l, r, b, t, (float)mNearAndFar.x, (float)mNearAndFar.y);


	mLightSpaceMatrix = lightProjection * lightView*mModel_matrix;
	mShadowMapCBData.lightSpaceMatrix = glm::transpose(mLightSpaceMatrix);

	mShadowMapCBData.world = glm::transpose(mModel_matrix);
	mDepthRenderBuffer->Bind();
	mDepthRenderBuffer->Clear(0, 0, 0, 0);
	mShader->useShader();
}

void CC3DShadowRender::RenderMesh(const CC3DMesh& pMesh, bool withAnimation)
{
	mShadowMapCBData.meshMat = glm::transpose(pMesh.m_MeshMat);
	mShadowMapCBData.meshMatInverse = glm::transpose(glm::inverse(pMesh.m_MeshMat));
	mShadowMapCBData.AnimationEnable = withAnimation;

	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ShadowSampler);
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	
	GetDynamicRHI()->UpdateConstantBuffer(mShadowMapCB, &mShadowMapCBData);
	GetDynamicRHI()->SetVSConstantBuffer(0, mShadowMapCB);
	GetDynamicRHI()->UpdateConstantBuffer(mShadowMapSkinMatCB, &mShadowMapSkinMatCBData);
	GetDynamicRHI()->SetVSConstantBuffer(1, mShadowMapSkinMatCB);

	GetDynamicRHI()->DrawPrimitive(pMesh.m_pGPUBuffer->VerticeBuffer, pMesh.m_pGPUBuffer->AtrributeCount, pMesh.m_pGPUBuffer->IndexBuffer);

}

void CC3DShadowRender::UnBind()
{
	mDepthRenderBuffer->UnBind();
}

void CC3DShadowRender::SetShaderResource(const std::string& path)
{

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_NORMAL, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
		{VERTEX_ATTRIB_BLEND_INDEX, FLOAT_C4},
		{VERTEX_ATTRIB_BLEND_WEIGHT, FLOAT_C4}
	};

	std::string path2 = path + "/Shader/3D/shadow_map_depth.fx";
	mShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path2, pAttribute2, 5, true);

	if (mBlurPass)
	{
		mBlurPass->SetShaderResource(path);
	}

	const int32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	mBlurPass->Init(SHADOW_WIDTH, SHADOW_HEIGHT);

	mDepthRenderBuffer = GetDynamicRHI()->CreateRenderTarget(SHADOW_WIDTH, SHADOW_HEIGHT, true, false, CC3DDynamicRHI::SFT_R32G32F);

	mShadowMapCB = GetDynamicRHI()->CreateConstantBuffer(sizeof(ShadowMapConstantBuffer));
	mShadowMapSkinMatCB = GetDynamicRHI()->CreateConstantBuffer(sizeof(ShadowMapSkinMatConstantBuffer));

}

void CC3DShadowRender::SetDirectionalLight(glm::vec3& dirLight)
{

}

void CC3DShadowRender::SetAnimation(CC3DSceneManage& scene, CC3DMesh* pMesh, int nModel)
{
	int skinID = pMesh->m_nSkinID;
	auto& Bone = scene.m_Skeleton[nModel]->m_BoneNodeArray[skinID];

	for (uint32 k = 0; k < Bone.size(); k++)
	{
		char strUniformName[100];
		strUniformName[0] = '\0';
		sprintf(strUniformName, "gBonesMatrix[%d]", k);
		mShadowMapSkinMatCBData.BoneMat[k] = glm::transpose(Bone[k].FinalMat);
	}
}

void CC3DShadowRender::ProcessBlur()
{
	if (mBlurPass)
	{
		mBlurPass->Process(mDepthRenderBuffer);
	}
}

std::shared_ptr<CC3DRenderTargetRHI> CC3DShadowRender::GetShadowMap()
{
	if (mBlurPass)
	{
		return mBlurPass->GetResult();
	}
	return nullptr;
}

