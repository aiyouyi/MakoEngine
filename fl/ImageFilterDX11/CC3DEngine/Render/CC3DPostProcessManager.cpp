#include "CC3DPostProcessManager.h"
#include <BaseDefine/Vectors.h>
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/RenderTargetRHI.h"
#include "EffectKernel/ShaderProgramManager.h"

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

		m_BloomFBO = GetDynamicRHI()->CreateRenderTarget(nWidth*m_scaleHW, nHeight*m_scaleHW, false, nullptr, CC3DDynamicRHI::SFT_A16R16G16B16F);

		for (int ni = 0; ni < DUAL_SAMPLE_COUNT; ni++)
		{
			bloomLevelFBO[ni] = GetDynamicRHI()->CreateRenderTarget(nWidth * m_scaleHW * pow(0.5, ni + 1), nHeight * m_scaleHW * pow(0.5, ni + 1), false, nullptr, CC3DDynamicRHI::SFT_A16R16G16B16F);
		}

	}
	else if (m_BloomFBO->GetWidth() != nWidth * m_scaleHW || m_BloomFBO->GetHeight() != nHeight * m_scaleHW)
	{
		m_BloomFBO = GetDynamicRHI()->CreateRenderTarget(nWidth*m_scaleHW, nHeight*m_scaleHW, false, nullptr, CC3DDynamicRHI::SFT_A16R16G16B16F);

		for (int ni = 0; ni < DUAL_SAMPLE_COUNT; ni++)
		{
			bloomLevelFBO[ni] = GetDynamicRHI()->CreateRenderTarget(nWidth * m_scaleHW * pow(0.5, ni + 1), nHeight * m_scaleHW * pow(0.5, ni + 1), false, nullptr, CC3DDynamicRHI::SFT_A16R16G16B16F);
		}

	}



}

void PostProcessManager::Process()
{

	DX11DoubleBuffer *pFBO = m_FBO_Ext;
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
		 Bloom(pFBO);
	}



	if (_enableFxaa)
	{
// 		pFBO->SyncAToB();
// 		pFBO->BindFBOA();
// 		Fxaa(pFBO->GetFBOB()->getTexture(), pFBO->GetWidth(), pFBO->GetHeight());
	}

}

void PostProcessManager::Fxaa(DX11Texture *input, int nWidth, int nHeight)
{
// 	glViewport(0, 0, nWidth, nHeight);
// 	pFxaaShader->Use();
// 	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// 	glViewport(0, 0, nWidth, nHeight);
// 
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, input);
// 	pFxaaShader->SetUniform1i("colorBuffer", 0);
// 
// 	pFxaaShader->SetUniform4f("resolution", nWidth, nHeight, 1.0 / nWidth, 1.0 / nHeight);
// 	pFxaaShader->SetUniform2f("clipControl", -0.5, 0.5);
// 	DrawTriangle();
// 	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessManager::Bloom(DX11DoubleBuffer * pFBO)
{
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

	m_BloomFBO->Bind();
	pScaleShader->useShader();
	pFBO->SetBShaderResource(0);
	float pParam[4];
	pParam[0] = m_bloomAlpha;
	pParam[1] = 1.0 / pFBO->GetWidth();
	pParam[2] = 1.0 / pFBO->GetHeight();
	SetParameter("param", pParam, 0, sizeof(Vector4));
	DrawTriangle();

	bloomLevelFBO[0]->Bind();
	pDualDownSample->useShader();
	GetDynamicRHI()->SetPSShaderResource(0, m_BloomFBO);

	pParam[0] = m_bloomRadius;
	pParam[1] = 0.5 / m_BloomFBO->GetWidth();
	pParam[2] = 0.5 / m_BloomFBO->GetHeight();
	SetParameter("param", pParam, 0, sizeof(Vector4));
	DrawTriangle();

	for (int ni = 1; ni < DUAL_SAMPLE_COUNT; ni++)
	{
		bloomLevelFBO[ni]->Bind();
		pDualDownSample->useShader();
		GetDynamicRHI()->SetPSShaderResource(0, bloomLevelFBO[ni - 1]);
		pParam[0] = m_bloomRadius;
		pParam[1] = 0.5 / bloomLevelFBO[ni - 1]->GetWidth();
		pParam[2] = 0.5 / bloomLevelFBO[ni - 1]->GetHeight();
		SetParameter("param", pParam, 0, sizeof(Vector4));
		DrawTriangle();
	}
	GetDynamicRHI()->SetBlendState(BlendADD, blendFactor, 0xffffffff);
	for (int ni = DUAL_SAMPLE_COUNT - 1; ni > 0; ni--)
	{

		bloomLevelFBO[ni-1]->Bind();
		pDualUpSample->useShader();
		GetDynamicRHI()->SetPSShaderResource(0, bloomLevelFBO[ni]);
		pParam[0] = m_bloomRadius;
		pParam[1] = 0.5 / bloomLevelFBO[ni]->GetWidth();
		pParam[2] = 0.5 / bloomLevelFBO[ni]->GetHeight();
		SetParameter("param", pParam, 0, sizeof(Vector4));
		DrawTriangle();
	}

	m_BloomFBO->Bind();
	pDualUpSample->useShader();
	GetDynamicRHI()->SetPSShaderResource(0, bloomLevelFBO[0]);
	pParam[0] = m_bloomRadius;
	pParam[1] = 0.5 / bloomLevelFBO[0]->GetWidth();
	pParam[2] = 0.5 / bloomLevelFBO[0]->GetHeight();
	SetParameter("param", pParam, 0, sizeof(Vector4));
	DrawTriangle();


	GetDynamicRHI()->SetBlendState(BlendADD2, blendFactor, 0xffffffff);
	m_FBO_Ext->BindFBOA();
	pShader->useShader();
	GetDynamicRHI()->SetPSShaderResource(0, m_BloomFBO);
	pParam[0] = m_bloomAlpha;
	SetParameter("param", pParam, 0, sizeof(Vector4));
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
		pScaleShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);
	}

	if (pShader == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/face2dTexture.fx";
		pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);

	}
	if (pDualDownSample == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/dualblur_downsample.fx";
		pDualDownSample = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);
	}

	if (pDualUpSample == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/dualblur_upsample.fx";
		pDualUpSample = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);
	}
}

