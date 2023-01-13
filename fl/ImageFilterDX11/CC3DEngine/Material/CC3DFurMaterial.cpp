#include "CC3DFurMaterial.h"
#include "Model/CC3DMesh.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "Toolbox/json.hpp"
#include "Toolbox/Render/MaterialTexRHI.h"

CC3DFurMaterial::CC3DFurMaterial()
{
	materialType = MaterialType::FUR;
}

CC3DFurMaterial::~CC3DFurMaterial()
{

}

void CC3DFurMaterial::InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig)
{

	mShader = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
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

		std::string ShaderPath = path + "/Shader/3D/FUR.fx";
		mShader->InitShader(ShaderPath, pAttribute, 6, true);
	}
	else
	{
		std::string  vspath = path + "/Shader/3D/fur.vs";
		std::string  fspath = path + "/Shader/3D/fur.fs";
		mShader->InitShader(vspath.c_str(), fspath.c_str());
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).Shader_ = mShader->GetGLProgram();
	}


	if (!noise_tex_file.empty() && std::ifstream(noise_tex_file).good())
	{
		noiseTexture = GetDynamicRHI()->CreateAsynTextureFromFile(noise_tex_file, false);
	}
}


void CC3DFurMaterial::PreRenderSet(CC3DMesh* pMesh)
{
	if (mShader != nullptr)
	{
		//draw solid black center
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurLength", 0.0f);
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("UVScale", 1.0f);
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("DrawSolid", 1);
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("UseLengthTex",0);


		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler);
		GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
		GetDynamicRHI()->SetPSShaderResource(1, m_BaseColorTexture);


		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).ApplyToAllBuffer();

		DrawTriangle(pMesh);
	}
}

void CC3DFurMaterial::RenderSet(CC3DMesh* pMesh)
{
	if (mShader != nullptr )
	{
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("DrawSolid", 0);
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurLength", furLength);
		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("UVScale", UVScale);

		GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("UseLengthTex", lengthTexture ? 1 : 0);

		for (int i = 0; i < numLayers; i++)
		{
			//float layer = (float)(i + 1) / numLayers;
			float furOffset = 1.0 / numLayers * (i + 1);
			GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurOffset", furOffset);

			GetDynamicRHI()->SetVSSamplerState(CC3DPiplelineState::WarpLinerSampler,1);
			GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler);
			GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
			noiseTexture->Bind(1);
			if (lengthTexture)
			{
				lengthTexture->Bind(2);
				lengthTexture->BindVS(3);
			}
			

			GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
			GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

			GET_SHADER_STRUCT_MEMBER(FurConstBuffer).ApplyToAllBuffer();

			DrawTriangle(pMesh);
		}
	}
}

void CC3DFurMaterial::UpdateModelConfig(CC3DImageFilter::EffectConfig* EffectConfig)
{
	CC3DPBRMaterial::UpdateModelConfig(EffectConfig);

	const auto& FurData = EffectConfig->FurData.ConfigData;

	numLayers = FurData.FurLevel;
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("UseToneMapping", FurData.UseToneMapping);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurGamma", FurData.FurGamma);

	//使用new PBR代表新版本 毛发长度生效
	if (EffectConfig->ModelConfig.UseNewPBR)
	{
		furLength = FurData.FurLength;
		UVScale = FurData.UVScale;
	}

	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("vGravity", FurData.vGravity);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("LightFilter", FurData.LightFilter);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurLightExposure", FurData.FurLightExposure);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurAmbientStrength", FurData.FurAmbientStrength);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FresnelLV", FurData.Fresnel);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("FurMask", FurData.FurMask);
	GET_SHADER_STRUCT_MEMBER(FurConstBuffer).SetParameter("Tming", FurData.Tming);

	noiseTexture = EffectConfig->FurData.noiseTex;
	lengthTexture = EffectConfig->FurData.lengthTex;
}

