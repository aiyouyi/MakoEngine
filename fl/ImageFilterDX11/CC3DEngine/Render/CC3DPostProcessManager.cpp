#include "CC3DPostProcessManager.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/RenderTargetRHI.h"
#include "Toolbox/Render/DoubleBufferRHI.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/ShaderRHI.h"


PostProcessManager::PostProcessManager()
{
}

PostProcessManager::~PostProcessManager()
{
}

void PostProcessManager::Init(int nWidth, int nHeight)
{
	if (!m_BloomFBO)
	{
		Vector3 arrCoords[VERT_NUM] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};
		mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, VERT_NUM, 3);
		mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);

		BlendADD = GetDynamicRHI()->CreateBlendState(CC3DBlendState::BP_ONE, CC3DBlendState::BP_ONE);
		BlendADD2 = GetDynamicRHI()->CreateBlendState(CC3DBlendState::BP_SRCALPHA, CC3DBlendState::BP_ONE);

		m_BloomFBO = GetDynamicRHI()->CreateRenderTarget(nWidth*m_scaleHW, nHeight*m_scaleHW, false, nullptr, CC3DTextureRHI::SFT_A16R16G16B16F);

		for (int ni = 0; ni < DUAL_SAMPLE_COUNT; ni++)
		{
			bloomLevelFBO[ni] = GetDynamicRHI()->CreateRenderTarget(nWidth * m_scaleHW * pow(0.5, ni + 1), nHeight * m_scaleHW * pow(0.5, ni + 1), false, nullptr, CC3DTextureRHI::SFT_A16R16G16B16F);
		}

	}
	else if (m_BloomFBO->GetWidth() != nWidth * m_scaleHW || m_BloomFBO->GetHeight() != nHeight * m_scaleHW)
	{
		m_BloomFBO = GetDynamicRHI()->CreateRenderTarget(nWidth*m_scaleHW, nHeight*m_scaleHW, false, nullptr, CC3DTextureRHI::SFT_A16R16G16B16F);

		for (int ni = 0; ni < DUAL_SAMPLE_COUNT; ni++)
		{
			bloomLevelFBO[ni] = GetDynamicRHI()->CreateRenderTarget(nWidth * m_scaleHW * pow(0.5, ni + 1), nHeight * m_scaleHW * pow(0.5, ni + 1), false, nullptr, CC3DTextureRHI::SFT_A16R16G16B16F);
		}

	}

}

void PostProcessManager::Process()
{
	//_enableBloom = false;
	if (m_bloomAlpha>0.001)
	{
		_enableBloom = true;
	}
	else
	{
		_enableBloom = false;
	}
	if (_enableBloom)
	{
		 Bloom();
	}

}


void PostProcessManager::Bloom()
{
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

	m_BloomFBO->Bind();
	pScaleShader->UseShader();

	GetDynamicRHI()->SetPSShaderResource(0, m_FBO_Ext->GetFBOTextureB());

	Vector4 Param;
	Param[0] = m_bloomAlpha;
	Param[1] = 1.0 / m_FBO_Ext->GetWidth();
	Param[2] = 1.0 / m_FBO_Ext->GetHeight();
	SetParameter("param", Param);
	DrawTriangle();

	bloomLevelFBO[0]->Bind();
	pDualDownSample->UseShader();
	GetDynamicRHI()->SetPSShaderResource(0, m_BloomFBO);

	Param[0] = m_bloomRadius;
	Param[1] = 0.5 / m_BloomFBO->GetWidth();
	Param[2] = 0.5 / m_BloomFBO->GetHeight();
	SetParameter("param", Param);
	DrawTriangle();

	for (int ni = 1; ni < DUAL_SAMPLE_COUNT; ni++)
	{
		bloomLevelFBO[ni]->Bind();
		pDualDownSample->UseShader();
		GetDynamicRHI()->SetPSShaderResource(0, bloomLevelFBO[ni - 1]);
		Param[0] = m_bloomRadius;
		Param[1] = 0.5 / bloomLevelFBO[ni - 1]->GetWidth();
		Param[2] = 0.5 / bloomLevelFBO[ni - 1]->GetHeight();
		SetParameter("param", Param);
		DrawTriangle();
	}
	GetDynamicRHI()->SetBlendState(BlendADD, blendFactor, 0xffffffff);
	for (int ni = DUAL_SAMPLE_COUNT - 1; ni > 0; ni--)
	{

		bloomLevelFBO[ni-1]->Bind();
		pDualUpSample->UseShader();
		GetDynamicRHI()->SetPSShaderResource(0, bloomLevelFBO[ni]);
		Param[0] = m_bloomRadius;
		Param[1] = 0.5 / bloomLevelFBO[ni]->GetWidth();
		Param[2] = 0.5 / bloomLevelFBO[ni]->GetHeight();
		SetParameter("param", Param);
		DrawTriangle();
	}

	m_BloomFBO->Bind();
	pDualUpSample->UseShader();
	GetDynamicRHI()->SetPSShaderResource(0, bloomLevelFBO[0]);
	Param[0] = m_bloomRadius;
	Param[1] = 0.5 / bloomLevelFBO[0]->GetWidth();
	Param[2] = 0.5 / bloomLevelFBO[0]->GetHeight();
	SetParameter("param", Param);
	DrawTriangle();


	GetDynamicRHI()->SetBlendState(BlendADD2, blendFactor, 0xffffffff);
	m_FBO_Ext->BindFBOA();
	pShader->UseShader();
	GetDynamicRHI()->SetPSShaderResource(0, m_BloomFBO);
	Param[0] = m_bloomAlpha;
	SetParameter("param", Param);
	DrawTriangle();
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
}

void PostProcessManager::DrawTriangle()
{
	GET_SHADER_STRUCT_MEMBER(ConstantBufferVec4).ApplyToAllBuffer();
	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);
}

void PostProcessManager::SetShaderResource(const std::string& path)
{
// 	if (pFxaaShader == nullptr)
// 	{
// 		pFxaaShader = std::make_shared<CCProgram>();
// 		std::string vs_depth = path + "/Shader/fxaa.vs";
// 		std::string fs_depth = path + "/Shader/fxaa.fs";
// 		pFxaaShader->CreatePorgramForFile(vs_depth.c_str(), fs_depth.c_str());
// 	}
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
	};
	
	if (pScaleShader == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/scale.fx";
		pScaleShader = GetDynamicRHI()->CreateShaderRHI();
		pScaleShader->InitShader(fsPath, pAttribute, 1);
	}

	if (pShader == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/face2dTexture.fx";
		pShader = GetDynamicRHI()->CreateShaderRHI();
		pShader->InitShader(fsPath, pAttribute, 1);

	}
	if (pDualDownSample == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/dualblur_downsample.fx";
		pDualDownSample = GetDynamicRHI()->CreateShaderRHI();
		pDualDownSample->InitShader(fsPath, pAttribute, 1);
	}

	if (pDualUpSample == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/dualblur_upsample.fx";
		pDualUpSample = GetDynamicRHI()->CreateShaderRHI();
		pDualUpSample->InitShader(fsPath, pAttribute, 1);
	}
}

