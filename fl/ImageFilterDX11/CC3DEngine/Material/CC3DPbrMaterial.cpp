#include "CC3DPbrMaterial.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "BaseDefine/json.hpp"
#include "CC3DMakeUp.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include "Material/EffectConfig.h"

CC3DPBRMaterial::CC3DPBRMaterial()
{
	materialType = MaterialType::PBR;

	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ConstBuffer.OutLineColor = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
}

CC3DPBRMaterial::~CC3DPBRMaterial()
{

}

void CC3DPBRMaterial::InitMaterial(uint32 MaterialIndex, const std::vector<std::shared_ptr<CC3DTextureRHI>>& ModelTexture)
{
	CC3DMaterial::InitMaterial(MaterialIndex, ModelTexture);
	//MaterialName
	if (MaterialName.find("FN") != std::string::npos)
	{
		SetFlattenNormal(true);
	}

	if (MaterialName.find("Kajiya") != std::string::npos)
	{
		SetEnableKajiya(true);
	}
}

void CC3DPBRMaterial::InitShaderProgram(const std::string& path, CC3DImageFilter::EffectConfig* EffectConfig)
{
	if (mShader)
	{
		return;
	}
	mShader = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute2[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
			{VERTEX_ATTRIB_NORMAL, FLOAT_C3},
			{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
			{VERTEX_ATTRIB_TANGENT,FLOAT_C4},
			{VERTEX_ATTRIB_BLEND_INDEX, FLOAT_C4},
			{VERTEX_ATTRIB_BLEND_WEIGHT, FLOAT_C4},
		};

		std::string PBRShader = path + "/Shader/3D/PBR2.fx";
		if (!EffectConfig->ModelConfig.UseNewPBR)
		{
			PBRShader = path + "/Shader/3D/PBR.fx";
		}
		mShader->InitShader(PBRShader, pAttribute2, 6, true);
	}
	else
	{
		std::string  vspath = path + "/Shader/3D/pbrTex.vs";
		std::string  fspath = path + "/Shader/3D/pbrTex.fs";
		mShader->InitShader(vspath.c_str(), fspath.c_str());
		GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(PBRSkinMat).Shader_ = mShader->GetGLProgram();
		GET_SHADER_STRUCT_MEMBER(PBRTexture).Shader_ = mShader->GetGLProgram();
	}

	MakeUp = std::make_shared<CC3DMakeUp>();
	MakeUp->InitShaderProgram(path);
}

void CC3DPBRMaterial::RenderSet(CC3DMesh* pMesh)
{
	if (EnableChangeColor)
	{
		SetTexture2D("albedoMap", MakeUp->GetOutPut());
	}
	else
	{
		SetTexture2D("albedoMap", m_BaseColorTexture);
	}

	SetTexture2D("normalMap", m_NormalTexture);
	SetTexture2D("roughness_metallicMap", m_MetallicRoughnessTexture);
	SetTexture2D("EmissMap", m_EmissiveTexture);
	SetTexture2D("aoMap", m_OcclusionTexture);
	if (SkinSpecularBRDF)
	{
		SetTexture2D("SkinSpecularBRDF", SkinSpecularBRDF);
	}
	if (PreSkinLut)
	{
		SetTexture2D("PreintegratedSkinLut", PreSkinLut);
	}
	if (BlurNormalMap)
	{
		SetTexture2D("BlurNormalMap", BlurNormalMap);
	}
	if (shiftMap)
	{
		SetTexture2D("shiftMap", shiftMap);
	}
	if (OutlineMask)
	{
		SetTexture2D("OutLineMask", OutlineMask);
	}

	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::MirrorLinerSampler);

	float variance = 0.15;
	SetParameter("_specularAntiAliasingVariance", variance);
	float threshold = 0.04;
	SetParameter("_specularAntiAliasingThreshold", threshold);
	SetParameter("FrontNoramlScale", 1.0f);

	if (EmissiveMaskTexture)
	{
		SetTexture2D("EmissiveMask", EmissiveMaskTexture);
	}
	
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("UseEmissiveMask", EmissiveMaskTexture ? 1 : 0);

	GET_SHADER_STRUCT_MEMBER(PBRToneMapping).ApplyToPSBuffer();
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer1).ApplyToPSBuffer();
	GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

	DrawTriangle(pMesh);

}

void CC3DPBRMaterial::RenderHSV(const std::unordered_map<std::string, Vector4>& HSVColors)
{
	auto ItColor = HSVColors.find(MaterialName);
	if (ItColor == HSVColors.end())
	{
		return;
	}

	if (ItColor->second != HSVColor)
	{

		HSVColor = ItColor->second;
		EnableChangeColor = true;
		if (!InitMakeUp)
		{
			MakeUp->Init(m_BaseColorTexture->GetWidth(), m_BaseColorTexture->GetHeight());
			InitMakeUp = true;
		}

		MakeUp->ChangeColor(HSVColor.x, HSVColor.y, HSVColor.z, HSVColor.w, m_BaseColorTexture, m_OcclusionTexture);
	}
}

void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(PBRTexture).SetTexture(str, TextureRHI);
}

void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{
	GET_SHADER_STRUCT_MEMBER(PBRTexture).SetTexture(str, RenderTargetRHI);
}

void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(PBRTexture).SetTexture(str, TextureRHI);
}

void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<MaterialTexRHI> TextureRHI)
{
	TextureRHI->UpdateTexture();
	SetTexture2D(str, TextureRHI->GetTextureRHI());
}

void CC3DPBRMaterial::SetParams(const std::string& Params)
{
	if (Params.empty())
	{
		return;
	}

	if (MaterialName.find("Kajiya") != std::string::npos)
	{
		SetEnableKajiya(true);
	}

	nlohmann::json Root = nlohmann::json::parse(Params);
	if (!Root.is_null())
	{
		auto& NormalInfo = Root["FlattenNormal"];
		if (!NormalInfo.is_null())
		{
			for (int ni = 0; ni < NormalInfo.size(); ni++)
			{
				nlohmann::json& nameJson = NormalInfo[ni];
				std::string str = nameJson["Name"].get<std::string>();
				if (str == MaterialName)
				{
					GET_CONSTBUFFER(PBRConstantBuffer).NormalIntensity = nameJson["Params"][0].get<float>();
					GET_CONSTBUFFER(PBRConstantBuffer).FrontNormalOffset = nameJson["Params"][1].get<float>();
					GET_CONSTBUFFER(PBRConstantBuffer).FrontNoramlScale = nameJson["Params"][2].get<float>();
					break;
				}
			}
		}
		auto& OutLineInfo = Root["OutLine"];
		if (!OutLineInfo.is_null())
		{
			for (int ni = 0; ni < OutLineInfo.size(); ni++)
			{
				nlohmann::json& nameJson = OutLineInfo[ni];
				std::string str = nameJson["Name"].get<std::string>();
				if (str == MaterialName)
				{
					GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor.x = nameJson["Params"][0].get<float>();
					GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor.y = nameJson["Params"][1].get<float>();
					GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor.z = nameJson["Params"][2].get<float>();
					GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor.w = nameJson["Params"][3].get<float>();
					break;
				}
			}
		}
		auto& KajiyaInfo = Root["Kajiya"];
		if (!KajiyaInfo.is_null())
		{
			GET_CONSTBUFFER(PBRConstantBuffer).PrimaryShift = KajiyaInfo[0].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).SecondaryShift = KajiyaInfo[1].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).SpecularPower = KajiyaInfo[2].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).ShiftU = KajiyaInfo[3].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecularScale = KajiyaInfo[4].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecularWidth = KajiyaInfo[5].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecualrColor.w = KajiyaInfo[6].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecualrColor.x = KajiyaInfo[7].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecualrColor.y = KajiyaInfo[8].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecualrColor.z = KajiyaInfo[9].get<float>();
		}
		auto& ShadowInfo = Root["ShadowColor"];
		if (!ShadowInfo.is_null())
		{
			GET_CONSTBUFFER(PBRConstantBuffer).shadowColor.x = ShadowInfo[0].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).shadowColor.y = ShadowInfo[1].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).shadowColor.z = ShadowInfo[2].get<float>();
			GET_CONSTBUFFER(PBRConstantBuffer).shadowColor.w = (1.0 - ShadowInfo[3].get<float>());
		}
		auto& OutlineWidthInfo = Root["OutLineWidth"];
		if (!OutlineWidthInfo.is_null())
		{
			GET_CONSTBUFFER(PBRConstantBuffer).OutlineWidth = OutlineWidthInfo.get<float>() / 10000.0f;
		}
	}
}

void CC3DPBRMaterial::UpdateModelConfig(CC3DImageFilter::EffectConfig* EffectConfig)
{
	auto& ModelConfig = EffectConfig->ModelConfig;

	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("roughnessRate", ModelConfig.RoughnessRate);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("reverseY", 0);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("ambientStrength", ModelConfig.AmbientStrength);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("u_EnbleRMGamma", ModelConfig.bEnableGammaRM);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("gamma", ModelConfig.Gamma);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("u_EnbleEmiss", ModelConfig.EnableEmiss);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("AoOffset", ModelConfig.AOOffset);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("BloomThresold", 1.0);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("BloomStrength", ModelConfig.Bloom.BloomStrength);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).SetParameter("HDRScale", ModelConfig.HDRScale);

	GET_SHADER_STRUCT_MEMBER(PBRToneMapping).SetParameter("ToneMappingType", ModelConfig.ToneMapping.ToneMappingType);
	GET_SHADER_STRUCT_MEMBER(PBRToneMapping).SetParameter("ToneMapping_Contrast", ModelConfig.ToneMapping.Contrast);
	GET_SHADER_STRUCT_MEMBER(PBRToneMapping).SetParameter("ToneMapping_Saturation", ModelConfig.ToneMapping.Saturation);
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer1).SetParameter("HDRContrast", ModelConfig.HDRContrast);

	auto EmissiveMaskTex = EffectConfig->EmissiveMaskTex;
	if (EmissiveMaskTex.count(MaterialName) == 1)
	{
		auto EmissiveMaskMat = EmissiveMaskTex[MaterialName];
		EmissiveMaskMat->UpdateTexture();
		EmissiveMaskTexture = EmissiveMaskMat->GetTextureRHI();
	}

}

void CC3DPBRMaterial::SetFlattenNormal(bool Enable)
{
	EnableFN = Enable;
}

void CC3DPBRMaterial::SetEnableKajiya(bool Enable)
{
	GET_CONSTBUFFER(PBRConstantBuffer).EnableKajiya = Enable;
}