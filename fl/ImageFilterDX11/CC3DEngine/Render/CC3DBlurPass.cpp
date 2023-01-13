#include "CC3DBlurPass.h"
#include <BaseDefine/Vectors.h>
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/DynamicRHI.h"

CC3DBlurPass::CC3DBlurPass()
{

}

CC3DBlurPass::~CC3DBlurPass()
{
	for (auto it : mDoubleBuffer)
	{
		it.reset();
	}
}

void CC3DBlurPass::Init(int nWidth, int nHeight, uint32_t format)
{
	if (mbInit)
	{
		return;
	}
	mWidth = nWidth;
	mHeight = nHeight;
	Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };

	unsigned short index[] =
	{
		0, 1, 2,
		1, 2, 3
	};
	
	mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, _countof(arrCoords), 3);
	mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);

	for (auto &it : mDoubleBuffer)
	{
		it = GetDynamicRHI()->CreateRenderTarget(nWidth, nHeight, false, nullptr, format);
	}
	mbInit = true;
}

void CC3DBlurPass::SetShaderResource(const std::string& path)
{
	if (mShader)
	{
		return;
	}

	mShader = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		};

		std::string fsPath = path + "/Shader/3D/Gaussianblur.fx";
		mShader->InitShader(fsPath, pAttribute, 1,false);
	}
	else
	{
		std::string vsPath = path + "/Shader/3D/Gaussianblur.vs";
		std::string psPath = path + "/Shader/3D/Gaussianblur.fs";
		mShader->InitShader(vsPath.c_str(), psPath.c_str());
		GET_SHADER_STRUCT_MEMBER(Gaussianblur).Shader_ = mShader->GetGLProgram();
	}

}

void CC3DBlurPass::Process(std::shared_ptr<CC3DRenderTargetRHI> input)
{

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

	int horizontal = true, first_iteration = true;
	int amount = 2;
	
	for (unsigned int i = 0; i < amount; i++)
	{
		std::shared_ptr<CC3DRenderTargetRHI> FrameBuffer = mDoubleBuffer[horizontal];
		FrameBuffer->Bind();
		mShader->UseShader();

		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

		GET_SHADER_STRUCT_MEMBER(Gaussianblur).SetParameter("horizontal", horizontal);
		GET_SHADER_STRUCT_MEMBER(Gaussianblur).SetTexture("image", first_iteration ? input : mDoubleBuffer[!horizontal]);
		GET_SHADER_STRUCT_MEMBER(Gaussianblur).ApplyToAllBuffer();

		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);

		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;

		FrameBuffer->UnBind();
	}
}

std::shared_ptr<CC3DRenderTargetRHI> CC3DBlurPass::GetResult()
{
	if (mDoubleBuffer.empty())
	{
		return nullptr;
	}
	return mDoubleBuffer[0];
}
