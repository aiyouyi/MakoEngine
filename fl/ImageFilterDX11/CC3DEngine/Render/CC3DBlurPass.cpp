#include "CC3DBlurPass.h"
#include <BaseDefine/Vectors.h>
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"

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

void CC3DBlurPass::Init(int nWidth, int nHeight)
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
	mContanstBuffer = GetDynamicRHI()->CreateConstantBuffer(sizeof(int)*4);

	for (auto &it : mDoubleBuffer)
	{
		it = GetDynamicRHI()->CreateRenderTarget(nWidth, nHeight, false, nullptr, CC3DDynamicRHI::SFT_R32G32F);
	}
	mbInit = true;
}

void CC3DBlurPass::SetShaderResource(const std::string& path)
{
	if (mShader)
	{
		return;
	}

	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
	};

	if (mShader == nullptr)
	{
		std::string fsPath = path + "/Shader/3D/Gaussianblur.fx";
		mShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fsPath, pAttribute, 1);
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
		std::shared_ptr<CC3DRenderTargetRHI> FameBuffer = mDoubleBuffer[horizontal];
		FameBuffer->Bind();

		mShader->useShader();

		GetDynamicRHI()->SetPSShaderResource(0, first_iteration ? input : mDoubleBuffer[!horizontal]);
		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

		GetDynamicRHI()->UpdateConstantBuffer(mContanstBuffer, &horizontal);
		GetDynamicRHI()->SetVSConstantBuffer(0, mContanstBuffer);
		GetDynamicRHI()->SetPSConstantBuffer(0, mContanstBuffer);

		GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);

		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;

		FameBuffer->UnBind();
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
