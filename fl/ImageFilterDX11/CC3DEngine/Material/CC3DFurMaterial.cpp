#include "CC3DFurMaterial.h"
#include "Model/CC3DMesh.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/RenderState/PiplelineState.h"


CC3DFurMaterial::CC3DFurMaterial()
{
	materialType = MaterialType::FUR;

	//colorTexture = new CC3DTexture();
}

CC3DFurMaterial::~CC3DFurMaterial()
{
	//SAFE_DELETE(colorTexture);
}

void CC3DFurMaterial::InitShaderProgram(std::string path)
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_NORMAL, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
		{VERTEX_ATTRIB_TANGENT,FLOAT_C4},
		{VERTEX_ATTRIB_BLEND_INDEX, FLOAT_C4},
		{VERTEX_ATTRIB_BLEND_WEIGHT, FLOAT_C4}
	};

	std::string fxpath = path + "/Shader/3D/FUR.fx";
	pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(fxpath, pAttribute, 6, true);


	fur_uniformbuffer = GetDynamicRHI()->CreateConstantBuffer(sizeof(FurConstBuffer));

	//´´½¨ÎÆÀí

	if (m_BaseColorTexture == nullptr)
	{
		m_BaseColorTexture = GetDynamicRHI()->CreateTextureFromFile(default_tex_file.c_str(), false);

	}
}


void CC3DFurMaterial::PreRenderSet(CC3DMesh* pMesh)
{
	if (pShader != nullptr)
	{
		//draw solid black center
		fur_constBuffer.furLength = 0.0;
		fur_constBuffer.uvScale = 1.0;
		fur_constBuffer.drawSolid = 1;
		fur_constBuffer.useLengthTex = 0;

		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler);
		GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
		GetDynamicRHI()->SetPSShaderResource(1, m_BaseColorTexture);

		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

		GetDynamicRHI()->UpdateConstantBuffer(fur_uniformbuffer, &fur_constBuffer);
		GetDynamicRHI()->SetVSConstantBuffer(2, fur_uniformbuffer);
		GetDynamicRHI()->SetPSConstantBuffer(2, fur_uniformbuffer);

		DrawTriangle(pMesh);
	}
}

void CC3DFurMaterial::RenderSet(CC3DMesh* pMesh)
{
	if (pShader != nullptr )
	{
		fur_constBuffer.drawSolid = 0;
		fur_constBuffer.furLength = furLength;
		fur_constBuffer.uvScale = UVScale;
		if (lengthTexture)
		{
			fur_constBuffer.useLengthTex = 1;
		}
		for (int i = 0; i < numLayers; i++)
		{
			float layer = (float)(i + 1) / numLayers;
			fur_constBuffer.furOffset = layer;

			GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler);
			GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
			GetDynamicRHI()->SetPSShaderResource(1, noiseTexture);
			GetDynamicRHI()->SetPSShaderResource(2, lengthTexture);

			GetDynamicRHI()->SetVSShaderResource(3, lengthTexture);
			GetDynamicRHI()->SetVSSamplerState(CC3DPiplelineState::WarpLinerSampler);

			GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
			GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

			GetDynamicRHI()->UpdateConstantBuffer(fur_uniformbuffer, &fur_constBuffer);
			GetDynamicRHI()->SetVSConstantBuffer(2, fur_uniformbuffer);
			GetDynamicRHI()->SetPSConstantBuffer(2, fur_uniformbuffer);

			DrawTriangle(pMesh);
		}
	}
}
