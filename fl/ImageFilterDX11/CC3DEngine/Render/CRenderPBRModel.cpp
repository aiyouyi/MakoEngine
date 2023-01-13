#include "CRenderPBRModel.h"
#include "Toolbox/HeaderModelForCull.h"
#include "CC3DEngine/Scene/CC3DSceneManage.h"
#include "CC3DEngine/Render/CC3DPbrRender.h"
#include "Toolbox/FSObject.h"

#include "CC3DEngine/Render/CC3DPostProcessManager.h"
#include "CC3DEngine/FaceExpressDrive/CC3DExpressDrive.h"
#include "Common/CC3DEnvironmentConfig.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "BaseDefine/json.hpp"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

#include "Toolbox/DXUtils/DX11DoubleBufferWrapper.h"
#include "CC3DEngine/Material/CC3DMaterial.h"
#include "CC3DEngine/Material/CC3DFurMaterial.h"
#include "Toolbox/Render/ShaderRHI.h"

CRenderPBRModel::CRenderPBRModel()
{
	m_Render = std::make_shared<CC3DImageFilter::CC3DPbrRender>();
	CC3DEnvironmentConfig::getInstance()->PbrRender = m_Render;
}

void CRenderPBRModel::Release()
{
	m_Render.reset();
	m_3DScene.reset();
	
	m_pHdrData.reset();
	for (int i = 0; i < m_MatrialVideo.size(); i++)
	{
		SAFE_DELETE(m_MatrialVideo[i].Animation);
	}

	SAFE_DELETE(m_postManager);
	SAFE_DELETE(m_ExpressDrive);
	m_DoubleBuffer.reset();
}

void CRenderPBRModel::SetHDRPath(const std::string& path)
{
	m_pHdrData.reset();
	if (EffectConfig)
	{
		EffectConfig->ModelConfig.HDRFileName = path;
	}
	if (m_Render)
	{
		m_Render->SetHDRPath(path);
	}
}

CRenderPBRModel::~CRenderPBRModel()
{
	Release();
}

bool CRenderPBRModel::Prepare(const std::string &resourePath)
{

	if (m_3DScene == NULL || m_3DScene->m_Model.size()==0)
	{
		return false;
	}


	m_Render->SetModeConfig(EffectConfig.get());
	m_Render->InitSecne(m_3DScene.get());
	m_Render->SetShaderResource(resourePath);

	if (!EffectConfig->DefOutLineMaskTex)
	{
		EffectConfig->DefOutLineMaskTex = GetDynamicRHI()->CreateAsynTexture(1, 1, 1, 1);
	}

	if (!m_postManager)
	{
		CreatePostProcessManager();
	}

	if (m_postManager!=NULL)
	{
		m_postManager->SetShaderResource(resourePath);
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		};

		if (pLDR2HDR == nullptr)
		{
			std::string fsPath = resourePath + "/Shader/3D/LDR2HDR.fx";
			pLDR2HDR = GetDynamicRHI()->CreateShaderRHI();
			pLDR2HDR->InitShader(fsPath, pAttribute, 1);
		}

		if (pHDR2LDR == nullptr)
		{
			std::string fsPath = resourePath + "/Shader/3D/HDR2LDR.fx";
			pHDR2LDR = GetDynamicRHI()->CreateShaderRHI();
			pHDR2LDR->InitShader(fsPath, pAttribute, 1);
		}
		Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};
		mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, 4, 3);
		mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);
	}

	if (m_ExpressDrive!=NULL)
	{
		auto path1 = resourePath + "/3DModels/model_74.endat";
		m_ExpressDrive->SetModelPath(path1.c_str());
	}
	lastTimeStamp = std::chrono::system_clock::now();
	//随机动画Timeline
	m_3DScene->ReSortAnimateTimeline(m_AnimateBlendTime);

	return true;
}

void CRenderPBRModel::PreRender(BaseRenderParam & RenderParam)
{
	if (m_3DScene == nullptr || m_3DScene->m_Model.size() == 0)
	{
		return;
	}

	if (!IsInit)
	{
		m_3DScene->DelayLoadModelResource(EffectConfig.get());
		if (m_pHdrData)
		{
			m_Render->m_RenderCube->setHDRData(m_pHdrData.get(), hdrWidth, hdrHeight);
		}
		IsInit = true;
	}
	
	m_Render->RenderCube();

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	int nFaceCount = RenderParam.GetFaceCount();

	EffectConfig->ModelConfig.EnableEmiss = 0;
	if (m_postManager != nullptr && EffectConfig->ModelConfig.Bloom.EnableBloom)
	{
		if (m_DoubleBuffer == nullptr || m_DoubleBuffer->GetWidth() != width || m_DoubleBuffer->GetHeight() != height)
		{
			
			m_DoubleBuffer = GetDynamicRHI()->CreateDoubleBuffer();
			if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
			{
				DX11DoubleBufferWrapper::InitDoubleBuffer(m_DoubleBuffer, width, height, true, true, CC3DTextureRHI::SFT_A16R16G16B16F);
			}
		}

		m_DoubleBuffer->BindFBOA();
		pLDR2HDR->UseShader();
		RHIResourceCast(RenderParam.GetDoubleBuffer().get())->SetAShaderResource(0);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
		EffectConfig->ModelConfig.EnableEmiss = 1;
	}
	else
	{
		m_DoubleBuffer = {};
	}

	if (nFaceCount > 0)
	{
		if (m_ExpressDrive != NULL && RenderParam.m_pBGRA_Src != NULL)
		{
			m_ExpressDrive->GetBlendshapeWeights(RenderParam.GetBGR_SRC(), width, height, RenderParam.GetFacePoint(0));
			std::vector<float> coeffs = m_ExpressDrive->GetCoeffs();
			if (coeffs.size() < BlendShapeCoffes)
			{
				return;
			}
			auto &BlendShapeName = m_ExpressDrive->GetBlendShapeNames();
			std::vector<float> headRotation = m_ExpressDrive->GetHeadRotation();

			int nModel = m_3DScene->m_Model.size();
			for (int i = 0; i < nModel; i++)
			{
				CC3DModel *pModel = m_3DScene->m_Model[i];
				int nMesh = pModel->m_ModelMesh.size();
				for (int j = 0; j < nMesh; j++)
				{
					CC3DMesh *pMesh = pModel->m_ModelMesh[j];
					if (pMesh->m_pBlendShapeName.size() > 0)
					{
						pMesh->GenVertWithCoeffs(coeffs, BlendShapeName);
					}
				}
			}
		}

	}

	m_3DScene->m_nSelectCamera = CamIndex;
	float fSecond = RunTime * 1.f / 1000;
	CC3DDirectionalLight *MainLight = (CC3DDirectionalLight *)m_3DScene->m_Light[0];
	MainLight->SetLightDirection(glm::vec3(EffectConfig->ModelConfig.LightDir[0], EffectConfig->ModelConfig.LightDir[1], EffectConfig->ModelConfig.LightDir[2]));
	MainLight->SetLightColor(EffectConfig->ModelConfig.LightColor[0], EffectConfig->ModelConfig.LightColor[1], EffectConfig->ModelConfig.LightColor[2]);
	MainLight->SetLightDiffuseIntensity(EffectConfig->ModelConfig.LightStrength);

	if (EffectConfig->ModelConfig.AnimationType == 0)
	{
		m_3DScene->play(fSecond * EffectConfig->ModelConfig.AnimateRate);

	}
	else if (EffectConfig->ModelConfig.AnimationType == 1)
	{
		m_3DScene->playOnce(fSecond * EffectConfig->ModelConfig.AnimateRate);
	}

	auto &modelMaterial = m_3DScene->m_Model[0]->m_ModelMaterial;
	for (int i = 0; i < modelMaterial.size(); i++)
	{
		modelMaterial[i]->RenderHSV(HSVColors);
		modelMaterial[i]->SetParams(RenderMaterialParams);
		modelMaterial[i]->UpdateModelConfig(EffectConfig.get());
		
		if (modelMaterial[i]->GET_CONSTBUFFER(PBRConstantBuffer).EnableKajiya == 1)
		{
			modelMaterial[i]->SetTexture2D("ShiftTex", EffectConfig->HairShiftTex);
		}

	}
	for (int i = 0; i < m_MatrialVideo.size(); i++)
	{

		for (int j = 0; j < modelMaterial.size(); j++)
		{
			if (m_MatrialVideo[i].MaterialName == modelMaterial[j]->MaterialName)
			{
				auto MaterialTex = m_MatrialVideo[i].Animation->GetTex(RunTime);
				MaterialTex->UpdateTexture();

				if (m_MatrialVideo[i].MaterialType=="emiss")
				{
					modelMaterial[j]->m_EmissiveTexture = MaterialTex->GetTextureRHI();
				}
				else
				{
					modelMaterial[j]->m_BaseColorTexture = MaterialTex->GetTextureRHI();
				}

				break;
			}
		}
	}


	if (EffectConfig->ModelConfig.bEnableShadow)
	{
		m_3DScene->m_ModelControl.UpdateModelMatrix();
		m_Render->RenderShadowMap();
	}
}

void CRenderPBRModel::Render(BaseRenderParam & RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();

	if (RenderBground)
	{
		m_Render->RenderBackGround();
	}

	m_Render->Render(width, height);

	auto end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - lastTimeStamp);
	CC3DEnvironmentConfig::getInstance()->deltaTime = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
	lastTimeStamp = end;

}

void CRenderPBRModel::PostRender(BaseRenderParam & RenderParam)
{
	if (m_3DScene == nullptr || m_3DScene->m_Model.size() == 0 )
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	if (m_postManager != nullptr && EffectConfig->ModelConfig.Bloom.EnableBloom)
	{
		m_postManager->Init(width, height);
		m_postManager->m_FBO_Ext = m_DoubleBuffer;
		m_postManager->m_bloomRadius = EffectConfig->ModelConfig.Bloom.Bloomradius*0.5;

		m_postManager->m_bloomAlpha = EffectConfig->ModelConfig.Bloom.BloomAlpha;
		if (EffectConfig->ModelConfig.Bloom.Bloomlooptime > 0)
		{
			int looptime = EffectConfig->ModelConfig.Bloom.Bloomlooptime * 2;

			float alpha = (RunTime % looptime)*2.f / looptime;
			if (alpha > 1)
			{
				alpha = 1;
			}
			m_postManager->m_bloomAlpha = (EffectConfig->ModelConfig.Bloom.BloomAlpha - EffectConfig->ModelConfig.Bloom.BloomBeginAlpha)*alpha + EffectConfig->ModelConfig.Bloom.BloomBeginAlpha;

		}

		m_postManager->_enableBloom = true;
		m_postManager->Process();

		pDoubleBuffer->BindFBOA();
		pHDR2LDR->UseShader();
		RHIResourceCast(m_DoubleBuffer.get())->SetAShaderResource(0);
		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
	}
}

void CRenderPBRModel::CreatePostProcessManager()
{
	m_postManager = new PostProcessManager();
}
