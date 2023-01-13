#include "CRenderPBRModelGL.h"

#include "BaseDefine/commonFunc.h"
#include "CC3DEngine/RenderGL/CC3DPostProcessManagerGL.h"
//#include "CC3DEngine/FaceExpressDrive/CC3DExpressDrive.h"
#include "Common/CC3DEnvironmentConfig.h"
#include "RenderGL/ShaderProgramManagerGL.h"
#include "GL/GLTexture2D.h"
#include <mutex>
#include "Material/CC3DFurMaterial.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Drawable.hpp"
//std::mutex g_mat_mutex;


CRenderPBRModelGL::CRenderPBRModelGL()
{
	//m_EffectPart = PBR_3D_MODEL;
	m_3DScene = NULL;
	m_Render = NULL;

	mrtFbo = nullptr;
	RenderSrcRHI = nullptr;
	RenderPosRHI = nullptr;
	HDRVerticeBuffer = nullptr;
	LDRVerticeBuffer = nullptr;
	pLDR2HDR = nullptr;
	pHDR2LDR = nullptr;

	InitBlendNameMap();

	m_Render = new CC3DPbrRenderGL();
}

CRenderPBRModelGL::~CRenderPBRModelGL()
{
	Release();
}

void CRenderPBRModelGL::Release()
{
	SAFE_DELETE(m_Render);
	m_3DScene.reset();
	m_pHdrData.reset();

	for (int i = 0; i < m_MatrialVideo.size(); i++)
	{
		SAFE_DELETE(m_MatrialVideo[i].Animation);
	}
	if (m_postManagerGL)
	{
		SAFE_DELETE(m_postManagerGL);
	}
	mrtFbo.reset();
	
	SAFE_DELETE(m_ExpressDrive);
	 //删除需要标记
}

void CRenderPBRModelGL::InitBlendNameMap()
{
	for (int i = 0; i < m_BlendShapeName2.size(); i++)
	{
		blendShapeMap[m_BlendShapeName2[i]] = i;
		m_BlendShapeName2[i][0] = m_BlendShapeName[i][0] + 'A' - 'a';
		blendShapeMap[m_BlendShapeName2[i]] = i;
	}
	for (int i = 0; i < m_BlendShapeName.size(); i++)
	{
		blendShapeMap[m_BlendShapeName[i]] = i;
	}

}

void CRenderPBRModelGL::BeginDrawPBRModelToFbo()
{
	if (mrtFbo)
	{
		RHIResourceCast(mrtFbo.get())->GetFBOABuffer()->bind();
	}
}

void CRenderPBRModelGL::ClearDepth()
{
	if (mrtFbo)
	{
		RHIResourceCast(mrtFbo.get())->GetFBOABuffer()->ClearDepth();
	}
}

void CRenderPBRModelGL::DrawToColorBuffers(int nCount)
{
	if (mrtFbo)
	{
		RHIResourceCast(mrtFbo.get())->GetFBOABuffer()->AttatchColorBuffers(nCount);
	}
}


bool CRenderPBRModelGL::Prepare(std::string &resourePath)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return false;
	}
	

	m_Render->SetModeConfig(&EffectConfig->ModelConfig);
	m_Render->InitSecne(m_3DScene.get());
	m_Render->SetShaderResource(resourePath);

	

	if (m_postManagerGL == nullptr)
	{
		CreatePostProcessManager();
	}

	if (m_postManagerGL != NULL)
	{
		m_postManagerGL->SetShaderResource(resourePath);

		pLDR2HDR = make_shared<CCProgram>();
		std::string  vspath = resourePath + "/Shader/3D/LDR2HDR.vs";
		std::string  fspath = resourePath + "/Shader/3D/LDR2HDR.fs";
		pLDR2HDR->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
		GET_SHADER_STRUCT_MEMBER(LDRToHDR).Shader_ = pLDR2HDR;

		pHDR2LDR = make_shared<CCProgram>();
		vspath = resourePath + "/Shader/3D/HDR2LDR.vs";
		fspath = resourePath + "/Shader/3D/HDR2LDR.fs";
		pHDR2LDR->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
		GET_SHADER_STRUCT_MEMBER(HDRToLDR).Shader_ = pHDR2LDR;
	}


	lastTimeStamp = std::chrono::system_clock::now();
	

	return true;
}

void CRenderPBRModelGL::PreRender(BaseRenderParam & RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}

	if (!IsInit)
	{
		m_3DScene->DelayLoadModelResource(EffectConfig.get());
		if (m_pHdrData)
		{
			m_Render->SetHDRData(m_pHdrData.get(), hdrWidth, hdrHeight);
		}
		IsInit = true;
	}

	m_Render->RenderCube();

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	int nFaceCount = RenderParam.GetFaceCount();

	if (m_postManagerGL != nullptr && EffectConfig->ModelConfig.Bloom.EnableBloom)
	{
		if (mrtFbo == nullptr || mrtFbo->GetWidth() != width  || mrtFbo->GetHeight() != height)
		{
			mrtFbo.reset();
			mrtFbo = GetDynamicRHI()->CreateDoubleBuffer();
			mrtFbo->InitDoubleBuffer(CC3DTextureRHI::SFT_A16R16G16B16F, width, height, true, true, false);
			//mrtFbo = new CCFrameBuffer(); //TODO:默认是RGBA16F
			///mrtFbo->init(width, height, true, GL_RGBA16F, GL_RGBA, GL_FLOAT,true);
			m_Render->m_FrameBuffer = RHIResourceCast(mrtFbo.get())->GetFBOABuffer(); //从外部设置 MRT FBO
		}
		CCFrameBuffer* GLFBO = RHIResourceCast(mrtFbo.get())->GetFBOABuffer();
		GLFBO->bind();
		GLFBO->AttatchColorBuffers(2);
		GLFBO->ClearColor();
		GLFBO->AttatchColorBuffers(1);

		static const GLfloat g_TextureCoordinate[] = { 
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f };

		//glViewport(0, 0, width, height);
		GetDynamicRHI()->SetViewPort(0, 0, width, height);
		GetDynamicRHI()->SetColorMask(1, 1, 1, 1);
		pLDR2HDR->Use();
		if (!HDRVerticeBuffer)
		{
			HDRVerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(1);
			HDRVerticeBuffer->CreateVertexBuffer((float*)&g_TextureCoordinate[0], 4, 2, 0);

		}
		RenderSrcRHI = RenderParam.GetDoubleBuffer()->GetFBOTextureA();
		GET_SHADER_STRUCT_MEMBER(LDRToHDR).SetTexture("inputImageTexture", RenderSrcRHI);

		GetDynamicRHI()->DrawPrimitive(HDRVerticeBuffer);

		GLFBO->unbind();
	
		EffectConfig->ModelConfig.EnableEmiss = true;
	}
	else
	{	
		if (mrtFbo == nullptr || mrtFbo->GetWidth() != width || mrtFbo->GetHeight() != height)
		{	
			//m_Render->m_FrameBuffer = mrtFbo = RHIResourceCast(RenderParam.GetDoubleBuffer().get())->GetFBOABuffer();
			mrtFbo = RenderParam.GetDoubleBuffer();
			m_Render->m_FrameBuffer = RHIResourceCast(mrtFbo.get())->GetFBOABuffer();
		}	
		EffectConfig->ModelConfig.EnableEmiss = false;

	}

 	if (nFaceCount > 0 && EffectConfig->ModelConfig.EnableExpress)
 	{

		std::vector<float> coeffs = RenderParam.GetExpressionCoffes();
		if (coeffs.size() < 52)
		{
			return;
		}

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
					pMesh->GenVertWithCoeffs(coeffs, blendShapeMap);
				}
			}
		}
 		
 	}

	m_3DScene->m_nSelectCamera = CamIndex;
	float fSecond = RenderParam.runtime * 1.f / 1000;
	CC3DDirectionalLight *MainLight = (CC3DDirectionalLight *)m_3DScene->m_Light[0];
	MainLight->SetLightDirection(glm::vec3(EffectConfig->ModelConfig.LightDir[0], EffectConfig->ModelConfig.LightDir[1], -EffectConfig->ModelConfig.LightDir[2]));
	MainLight->SetLightDiffuseIntensity(EffectConfig->ModelConfig.LightStrength);
	MainLight->SetLightAmbientIntensity(EffectConfig->ModelConfig.AmbientStrength);

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
		//shader没有use，还会使用上一次的shader
		modelMaterial[i]->UseShader();
		modelMaterial[i]->UpdateModelConfig(EffectConfig.get());

	}

	{
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - lastTimeStamp);
		CC3DEnvironmentConfig::getInstance()->deltaTime = double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
		lastTimeStamp = end;
	}

	if (EffectConfig->ModelConfig.bEnableShadow == 1)
	{
		m_3DScene->m_ModelControl.UpdateModelMatrix();
		m_Render->OutRenderShadowMap();
	}
}

void CRenderPBRModelGL::Render(BaseRenderParam& RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();

	m_Render->Render(width, height);

}

void CRenderPBRModelGL::PostRender(BaseRenderParam& RenderParam)
{
	if (m_3DScene == NULL || m_3DScene->m_Model.size() == 0)
	{
		return;
	}
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	if (m_postManagerGL != NULL && EffectConfig->ModelConfig.Bloom.EnableBloom)
	{
		GetDynamicRHI()->SetColorMask(1, 1, 1, 1);
		m_postManagerGL->Init(width, height);
		m_postManagerGL->m_FBO_Ext = m_Render->m_FrameBuffer;
		m_postManagerGL->m_bloomRadius = EffectConfig->ModelConfig.Bloom.Bloomradius;
		m_postManagerGL->m_bloomAlpha = EffectConfig->ModelConfig.Bloom.BloomAlpha;
		if (EffectConfig->ModelConfig.Bloom.Bloomlooptime > 0)
		{
			int looptime = EffectConfig->ModelConfig.Bloom.Bloomlooptime * 2;

			float alpha = (RenderParam.runtime % looptime)*2.f / looptime;
			if (alpha > 1)
			{
				alpha = 1;
			}
			m_postManagerGL->m_bloomAlpha = (EffectConfig->ModelConfig.Bloom.BloomAlpha - EffectConfig->ModelConfig.Bloom.BloomBeginAlpha)*alpha + EffectConfig->ModelConfig.Bloom.BloomBeginAlpha;

		}

		m_postManagerGL->_enableBloom = true;
		uint32 backbufferID = m_postManagerGL->Process();
		GetDynamicRHI()->SetColorMask(1, 1, 1, 0);
		pDoubleBuffer->BindFBOA();
		
		static const GLfloat g_TextureCoordinate[] = { 0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f
		};
		GetDynamicRHI()->SetViewPort(0, 0, pDoubleBuffer->GetWidth(), pDoubleBuffer->GetHeight());
		pHDR2LDR->Use();
		
		if (!LDRVerticeBuffer)
		{
			LDRVerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(1);
			LDRVerticeBuffer->CreateVertexBuffer((float*)&g_TextureCoordinate[0], 4, 2, 0);

			RenderPosRHI = GetDynamicRHI()->CreateTexture();
		}
		RenderPosRHI->AttatchTextureId(backbufferID);
		GET_SHADER_STRUCT_MEMBER(HDRToLDR).SetTexture("inputImageTexture", RenderPosRHI);

		GetDynamicRHI()->DrawPrimitive(LDRVerticeBuffer);
		
		
	}
	GetDynamicRHI()->SetColorMask(1, 1, 1, 1);
	pDoubleBuffer->unBindFBOA();

}

void CRenderPBRModelGL::CreatePostProcessManager()
{
	m_postManagerGL = new PostProcessManagerGL();
}

