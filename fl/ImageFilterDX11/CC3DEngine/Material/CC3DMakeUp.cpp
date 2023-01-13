#include "CC3DMakeUp.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/RenderTargetRHI.h"

CC3DMakeUp::CC3DMakeUp()
{

}

CC3DMakeUp::~CC3DMakeUp()
{

}

void CC3DMakeUp::Init(int width, int height)
{
	Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };

	unsigned short index[] =
	{
		0, 1, 2,
		1, 2, 3
	};

	mVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, _countof(arrCoords), 3);
	mIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);

	mOutPut = GetDynamicRHI()->CreateRenderTarget(width, height, false, nullptr, CC3DTextureRHI::SFT_A8R8G8B8);
}

void CC3DMakeUp::InitShaderProgram(const std::string& path)
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
		std::string fsPath = path + "/Shader/3D/changecolor.fx";
		mShader->InitShader(fsPath, pAttribute, 1);
	}
	else
	{
	}

}

void CC3DMakeUp::ChangeColor(float r, float g, float b, float a, std::shared_ptr<CC3DTextureRHI> pTexture, std::shared_ptr<CC3DTextureRHI> pMaskTexture /*= nullptr*/, CC3DMakeUpType type /*= CC3D_EYEBROW*/)
{
	if (!pTexture || !mShader)
	{
		return;
	}


	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

	mOutPut->Bind();
	mOutPut->Clear(0, 0, 0, 0);
	mShader->UseShader();

	GET_CONSTBUFFER(MakUpConstantBuffer).HSVColors = Vector4(r, g, b, a);
	GET_CONSTBUFFER(MakUpConstantBuffer).bUseMask = pMaskTexture ? 1 : 0;
	GET_SHADER_STRUCT_MEMBER(MakUpConstantBuffer).ApplyToPSBuffer();

	GET_SHADER_STRUCT_MEMBER(MakUpTexture).SetTexture("InputImage", pTexture);
	if (pMaskTexture)
	{
		GET_SHADER_STRUCT_MEMBER(MakUpTexture).SetTexture("MaskTexture", pMaskTexture);
	}

	GetDynamicRHI()->DrawPrimitive(mVertexBuffer, mIndexBuffer);

	mOutPut->UnBind();
}

