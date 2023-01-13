#include "CC3DShadowRender.h"
#include "Model/CC3DMesh.h"
#include "Scene/CC3DSceneManage.h"
#include "Scene/CC3DCamera.h"
#include "Effect/CC3DShadowMap.h"
#include "Effect/CC3DShadowMapManger.h"
#include "CC3DBlurPass.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/DynamicRHI.h"


CC3DShadowRender::CC3DShadowRender()
{
	mBlurPass = std::make_shared<CC3DBlurPass>();
}

CC3DShadowRender::~CC3DShadowRender()
{
	mBlurPass.reset();
	mDepthRenderBuffer.reset();
}

void CC3DShadowRender::Bind(CC3DImageFilter::CC3DSceneManage& scene_manager, CC3DImageFilter::ShadowMapManager& shadowManager)
{
	setModelMatrix(scene_manager.m_ModelControl.GetModelMatrix());

	CC3DDirectionalLight *pLight = (CC3DDirectionalLight*)scene_manager.m_Light[0];
	if (pLight != nullptr )
	{
		mLightDir = pLight->m_LightDirection;
		mLightDir = glm::normalize(mLightDir);
		
	}
	
	mLightLookAt = glm::vec3(0.0f); //此处没有考虑相机平移的情况
	
	mLightUp = glm::vec3(0.0f, 1.0, 0.0f);
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		mLightPos = mLightLookAt + (mLightDir * LIGHT_DISTANCE);
	}
	else
	{
		mLightPos = mLightLookAt + (-mLightDir * LIGHT_DISTANCE);
	}

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

	float l = minPoint.x;
	float b = minPoint.y;
	float n = minPoint.z;
	float r = maxPoint.x;
	float t = maxPoint.y;
	float f = maxPoint.z;

	glm::mat4 lightProjection;

	auto& shadow_map = *shadowManager.getShadowMap(0);
	mNearAndFar = glm::vec2(shadow_map.lsNear-0.1f, shadow_map.lsFar+0.1f);
	lightProjection = glm::ortho(l, r, b, t, (float)mNearAndFar.x, (float)mNearAndFar.y);


	mLightSpaceMatrix = lightProjection * lightView * mModel_matrix;

	mDepthRenderBuffer->Bind();
	mDepthRenderBuffer->Clear(0, 0, 0, 0);
	mShader->UseShader();

	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).SetParameter("lightSpaceMatrix", mLightSpaceMatrix);
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).SetParameter("model", mModel_matrix);

}

void CC3DShadowRender::RenderMesh(const CC3DMesh& pMesh, int withAnimation)
{
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).SetParameter("meshMat", pMesh.m_MeshMat);
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).SetParameter("meshMatInverse", glm::inverse(pMesh.m_MeshMat));
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).SetParameter("AnimationEnable", withAnimation);
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).SetParameter("alpha", ProjectShadow);

	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ShadowSampler);
	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateEnable);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).ApplyToAllBuffer();
	GET_SHADER_STRUCT_MEMBER(ShadowMapConst2).ApplyToVSBuffer();

	GetDynamicRHI()->DrawPrimitive(pMesh.m_pGPUBuffer->VerticeBuffer, pMesh.m_pGPUBuffer->AtrributeCount, pMesh.m_pGPUBuffer->IndexBuffer);

}

void CC3DShadowRender::UnBind()
{
	mDepthRenderBuffer->UnBind();
}

void CC3DShadowRender::SetShaderResource(const std::string& path)
{
	if (mShader)
	{
		return;
	}

	mShader = GetDynamicRHI()->CreateShaderRHI();

	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute2[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
			{VERTEX_ATTRIB_NORMAL, FLOAT_C3},
			{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
			{VERTEX_ATTRIB_TANGENT,FLOAT_C4},
			{VERTEX_ATTRIB_BLEND_INDEX, FLOAT_C4},
			{VERTEX_ATTRIB_BLEND_WEIGHT, FLOAT_C4},
		};

		std::string fsPath = path + "/Shader/3D/shadow_map_depth.fx";
		mShader->InitShader(fsPath, pAttribute2, 6, true);
	}
	else
	{
		std::string vs_depth = path + "/Shader/3D/shadow_map_depth.vs";
		std::string fs_depth = path + "/Shader/3D/shadow_map_depth.fs";
		mShader->InitShader(vs_depth.c_str(), fs_depth.c_str());
		GET_SHADER_STRUCT_MEMBER(ShadowMapConst1).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(ShadowMapConst2).Shader_ = mShader->GetGLProgram();
	}

	if (mBlurPass)
	{
		mBlurPass->SetShaderResource(path);
	}

	const int32_t SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		mDepthRenderBuffer = GetDynamicRHI()->CreateRenderTarget(SHADOW_WIDTH, SHADOW_HEIGHT, true, nullptr, CC3DTextureRHI::SFT_A32R32G32B32F);
		mBlurPass->Init(SHADOW_WIDTH, SHADOW_HEIGHT, CC3DTextureRHI::SFT_A32R32G32B32F);
	}
	else
	{
		mDepthRenderBuffer = GetDynamicRHI()->CreateRenderTarget(SHADOW_WIDTH, SHADOW_HEIGHT, true, nullptr, CC3DTextureRHI::SFT_R32G32B32F);
		mBlurPass->Init(SHADOW_WIDTH, SHADOW_HEIGHT, CC3DTextureRHI::SFT_R32G32B32F);
	}
	
}

void CC3DShadowRender::SetDirectionalLight(glm::vec3& dirLight)
{

}

void CC3DShadowRender::SetAnimation(CC3DImageFilter::CC3DSceneManage& scene, CC3DMesh* pMesh, int nModel)
{
	int skinID = pMesh->m_nSkinID;
	auto& Bone = scene.m_Skeleton[nModel]->m_BoneNodeArray[skinID];

	//
	for (uint32 k = 0; k < Bone.size(); k++)
	{
		GET_SHADER_STRUCT_MEMBER(ShadowMapConst2).SetArraySingleElementParamter("gBonesMatrix", Bone[k].FinalMat,k);
	}

	//for (uint32 k = 0; k < Bone.size(); k++)
	//{
	//	char strUniformName[100];
	//	strUniformName[0] = '\0';
	//	sprintf(strUniformName, "gBonesMatrix[%d]", k);
	//	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	//	{
	//		GET_CONSTBUFFER(ShadowMapConst2).BoneMat[k] = glm::transpose(Bone[k].FinalMat);
	//	}
	//	else
	//	{
	//		GET_SHADER_STRUCT_MEMBER(ShadowMapConst2).SetMatrix4Parameter(strUniformName, &Bone[k].FinalMat[0][0], false, 1);
	//	}
	//	
	//}
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

