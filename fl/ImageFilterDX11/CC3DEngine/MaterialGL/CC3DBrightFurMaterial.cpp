#include "CC3DBrightFurMaterial.h"
#include "RenderGL/ShaderProgramManagerGL.h"

#include "Model/CC3DMesh.h"
#include "ToolBox/Render/TextureRHI.h"
#include <fstream>
#include "json.hpp"
#include "RenderState/PiplelineState.h"
#include "Common/CC3DEnvironmentConfig.h"
using namespace nlohmann;

CC3DBrightFurMaterial::CC3DBrightFurMaterial()
{
	materialType = MaterialTypeGL::DYFUR;
	//texture_map = { {"u_FurColorTex", 0},
	//{ "u_FurStyleMask", 1 },
	//{ "lengthTexture", 2 } };
}

CC3DBrightFurMaterial::~CC3DBrightFurMaterial()
{

}

void CC3DBrightFurMaterial::InitShaderProgram(std::string path)
{
	mShader = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{

	}
	else
	{
		std::string  vspath = path + "/Shader/3D/brightfur.vs";
		std::string  fspath = path + "/Shader/3D/brightfur.fs";
		mShader->InitShader(vspath.c_str(), fspath.c_str());
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).Shader_ = mShader->GetGLProgram();
	}
	//std::string  vspath = path + "/Shader/3D/brightfur.vs";
	//std::string  fspath = path + "/Shader/3D/brightfur.fs";
	//pShader = ShaderProgramManagerGL::GetInstance()->GetOrCreateShaderByPath(vspath, fspath);

	if (!noise_tex_file.empty() && std::ifstream(noise_tex_file).good())
	{
		noiseTexture = GetDynamicRHI()->CreateAsynTextureFromFile(noise_tex_file, false);
	}

	if (m_BaseColorTexture == nullptr)
	{
		m_BaseColorTexture = GetDynamicRHI()->CreateTextureFromFile(default_tex_file, false);
	}
}

void CC3DBrightFurMaterial::RenderSet(CC3DMesh* pMesh)
{
	if (mShader != nullptr)
	{
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurLength", FurLength);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_AOColor", AOColor);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_LightFilter", LightFilter);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_Roughness", Roughness);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_StrandSpecColor", StrandSpecColor);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_StrandExpHigh", StrandExpHigh);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_StrandExpLow", StrandExpLow);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_StrandSpecColor_low", StrandSpecColor_low);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_StrandShift", StrandShift);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_UVoffset", UVOffset);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_ForcePow", ForcePow);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_Force", Force);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("lightIntensity", lightIntensity);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("vGravity", vGravity);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("vGravityStength", 0.5);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("drawSolid", 0);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_EnvironmentColor", EnvironmentColor);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_IBLExposure", IBLExposure);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurColor", FurColor);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurMaskScale", FurMaskScale);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurThickness", FurThickness);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurThickness_B", FurThickness_B);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurThickness_T", FurThickness_T);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurThickness_B2T", FurThickness_B2T);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurFeather", FurFeather);

		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler);
		GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
		noiseTexture->Bind(1);

		for (int i = 0; i < numLayers; ++i)
		{
			float furOffset = 0.75 / numLayers * (i + 1);
			GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("_FURLEVEL", furOffset);
			GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
			GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

			GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).ApplyToAllBuffer();

			DrawTriangle(pMesh);
		}


		//for (int i = 0; i < numLayers; ++i)
		//{
		//	float furOffset = 0.75 / numLayers * (i + 1);
		//	pShader->SetUniform1f("u_FurLength", FurLength);
		//	pShader->SetUniform1f("_FURLEVEL", furOffset);
		//	pShader->SetUniform4f("u_AOColor", AOColor.x, AOColor.y, AOColor.z, AOColor.w);
		//	pShader->SetUniform1f("u_LightFilter", LightFilter);
		//	pShader->SetUniform1f("u_Roughness", Roughness);
		//	pShader->SetUniform4f("u_StrandSpecColor", StrandSpecColor.x, StrandSpecColor.y, StrandSpecColor.z, StrandSpecColor.w);
		//	pShader->SetUniform1f("u_StrandExpHigh", StrandExpHigh);
		//	pShader->SetUniform1f("u_StrandExpLow", StrandExpLow);
		//	pShader->SetUniform4f("u_StrandSpecColor_low", StrandSpecColor_low.x, StrandSpecColor_low.y, StrandSpecColor_low.z, StrandSpecColor_low.w);
		//	pShader->SetUniform2f("u_StrandShift", StrandShift.x, StrandShift.y);
		//	pShader->SetUniform3f("u_UVoffset", UVOffset.x, UVOffset.y, UVOffset.z);
		//	pShader->SetUniform1f("u_ForcePow", ForcePow);
		//	pShader->SetUniform3f("u_Force", Force.x, Force.y, Force.z);
		//	pShader->SetUniform1f("lightIntensity", lightIntensity);
		//	pShader->SetUniform3f("vGravity", vGravity.x, vGravity.y, vGravity.z);
		//	pShader->SetUniform1f("vGravityStength", 0.5);
		//	pShader->SetUniform1i("drawSolid", false);
		//	pShader->SetUniform4f("u_EnvironmentColor", EnvironmentColor.x, EnvironmentColor.y, EnvironmentColor.z, EnvironmentColor.w);
		//	pShader->SetUniform1f("u_IBLExposure", IBLExposure);
		//	pShader->SetUniform4f("u_FurColor", FurColor.x, FurColor.y, FurColor.z, FurColor.w);
		//	pShader->SetUniform1f("u_FurMaskScale", FurMaskScale);
		//	pShader->SetUniform1f("u_FurThickness", FurThickness);
		//	pShader->SetUniform1f("u_FurThickness_B", FurThickness_B);
		//	pShader->SetUniform1f("u_FurThickness_T", FurThickness_T);
		//	pShader->SetUniform1f("u_FurThickness_B2T", FurThickness_B2T);
		//	pShader->SetUniform1f("u_FurFeather", FurFeather);

		//	SetTexture2D("u_FurColorTex", m_BaseColorTexture);
		//	SetTexture2D("u_FurStyleMask", noiseTexture);

		//	DrawTriangle(pMesh);
		//}
	}

}

void CC3DBrightFurMaterial::PreRenderSet(CC3DMesh* pMesh)
{
	if (mShader != nullptr)
	{
		////draw solid black center
		//{
		//	pShader->SetUniform1f("u_FurLength", 0.0);
		//	//pShader->SetUniform1f("UVScale", 1.0);
		//	pShader->SetUniform1i("drawSolid", true);
		//	SetTexture2D("u_FurColorTex", m_BaseColorTexture);
		//	SetTexture2D("u_FurStyleMask", m_BaseColorTexture);

		//	DrawTriangle(pMesh);
		//}

		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("u_FurLength", 0.0);
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("drawSolid", 1);
		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::WarpLinerSampler);
		GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
		GetDynamicRHI()->SetPSShaderResource(1, m_BaseColorTexture);
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).ApplyToAllBuffer();

		DrawTriangle(pMesh);
	}
}

void CC3DBrightFurMaterial::LoadConfig(const std::string config_file)
{
	if (config_file.empty())
	{
		return;
	}

	json root;
	std::ifstream ifile(config_file);
	ifile >> root;
	auto mtl_json = root["FurMaterial"];
	if (!mtl_json.is_null())
	{
		//for (int ni = 0; ni < fur_mtl.size(); ni++)
		{
			//json mtl_json = fur_mtl[ni];
			noise_tex_file = mtl_json["NoiseTexture"].get<std::string>();
			default_tex_file = mtl_json["DefaultTexture"].get<std::string>();
			//length_tex_file = mtl_json["LengthTexture"].get<std::string>();
			//flow_tex_file = mtl_json["FlowMap"].get<std::string>();
			FurLength = mtl_json["FurLength"].get<double>();
			numLayers = mtl_json["FurLevel"].get<int>();
			//uvScale = mtl_json["UVScale"].get<double>();
			//vForce.x = mtl_json["Gravity"][0];
			//vForce.y = mtl_json["Gravity"][1];
			//vForce.z = mtl_json["Gravity"][2];
			//lightFilter = mtl_json["LightFilter"].get<double>();
			//furLightExposure = mtl_json["FurLightExposure"].get<double>();
			//fresnelLV = mtl_json["FresnelLV"].get<double>();
			//furMask = mtl_json["FurMask"].get<double>();
			//tming = mtl_json["Tming"].get<double>();

			////�������Ը߹����
			//mainSpecShift = mtl_json["MainSpecShift"].get<double>();
			//secSpecShift = mtl_json["SecSpecShift"].get<double>();
			//mainSpecSmooth = mtl_json["MainSpecSmooth"].get<double>();
			//secSpecSmooth = mtl_json["SecSpecSmooth"].get<double>();

			//mainSpecColor.x = mtl_json["MainSpecColor"][0];
			//mainSpecColor.y = mtl_json["MainSpecColor"][1];
			//mainSpecColor.z = mtl_json["MainSpecColor"][2];

			//secSpecColor.x = mtl_json["SecSpecColor"][0];
			//secSpecColor.y = mtl_json["SecSpecColor"][1];
			//secSpecColor.z = mtl_json["SecSpecColor"][2];

		}
	}
}

void CC3DBrightFurMaterial::LoadEnvironmentConfig()
{
	nlohmann::json root = nlohmann::json::parse(CC3DEnvironmentConfig::getInstance()->FurData.FurJsonConfig);
	auto mtl_json = root["FurMaterial"];
	{
		GET_SHADER_STRUCT_MEMBER(BrightFurConstBuffer).SetParameter("FurLength", mtl_json["FurLength"].get<float>());

		FurLength = mtl_json["FurLength"].get<float>();
		numLayers = mtl_json["FurLevel"].get<int>();

		noiseTexture = CC3DEnvironmentConfig::getInstance()->FurData.noiseTex;
	}
}
