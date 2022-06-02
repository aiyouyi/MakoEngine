#include "CC3DPbrMaterial.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "BaseDefine/json.hpp"

CC3DPBRMaterial::CC3DPBRMaterial()
{
	materialType = MaterialType::PBR;
	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ConstBuffer.OutLineColor = Vector4(0.2f, 0.2f, 0.2f, 1.0f);
}

CC3DPBRMaterial::~CC3DPBRMaterial()
{

}

void CC3DPBRMaterial::InitShaderProgram(std::string path)
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

	std::string path2 = path + "/Shader/3D/PBR.fx";
	//DXUtils::CompileShaderWithFile(m_pShaderSkinPBR, (char*)path2.c_str(), pAttribute2, 5);
	pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path2, pAttribute2, 6, true);

}

void CC3DPBRMaterial::RenderSet(CC3DMesh* pMesh)
{
	GetDynamicRHI()->SetPSShaderResource(0, m_BaseColorTexture);
	GetDynamicRHI()->SetPSShaderResource(1, m_NormalTexture);
	GetDynamicRHI()->SetPSShaderResource(2, m_MetallicRoughnessTexture);
	GetDynamicRHI()->SetPSShaderResource(3, m_EmissiveTexture);
	GetDynamicRHI()->SetPSShaderResource(8, m_OcclusionTexture);
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::MirrorLinerSampler);

	GET_SHADER_STRUCT_MEMBER(PBRConstantBuffer).ApplyToAllBuffer();
	GET_SHADER_STRUCT_MEMBER(PBRSkinMat).ApplyToVSBuffer();

	DrawTriangle(pMesh);
}


void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{
	GET_SHADER_STRUCT_MEMBER(PBRTexture).SetTexture(str, RenderTargetRHI);
}

void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(PBRTexture).SetTexture(str, TextureRHI);
}

void CC3DPBRMaterial::SetTexture2D(const char* str, std::shared_ptr<CC3DCubeMapRHI> TextureRHI)
{
	GET_SHADER_STRUCT_MEMBER(PBRTexture).SetTexture(str, TextureRHI);
}

void CC3DPBRMaterial::SetParams(const std::string& Params)
{
	//Root["Normal"] = { FrontNoramlScale,FrontNormalOffset,NormalIntensity };
//Root["Kajiya"] = { PrimaryShift ,SecondaryShift ,SpecularPower ,ShiftU ,KajiyaSpecularScale ,KajiyaSpecularWidth };
	nlohmann::json Root = nlohmann::json::parse(Params);
	auto& NormalInfo = Root["Normal"];
	GET_CONSTBUFFER(PBRConstantBuffer).FrontNoramlScale = NormalInfo[0].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).FrontNormalOffset = NormalInfo[1].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).NormalIntensity = NormalInfo[2].get<float>();

	auto& KajiyaInfo = Root["Kajiya"];
	GET_CONSTBUFFER(PBRConstantBuffer).PrimaryShift = KajiyaInfo[0].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).SecondaryShift = KajiyaInfo[1].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).SpecularPower = KajiyaInfo[2].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).ShiftU = KajiyaInfo[3].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecularScale = KajiyaInfo[4].get<float>();
	GET_CONSTBUFFER(PBRConstantBuffer).KajiyaSpecularWidth = KajiyaInfo[5].get<float>();

	if (GET_CONSTBUFFER(PBRConstantBuffer).EnableRenderOutLine)
	{
		if (MaterialName.find("Hair") != std::string::npos)
		{
			auto& HairColor = Root["HairOutLineColor"];
			GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor = Vector4(HairColor[0].get<float>(), HairColor[1].get<float>(), HairColor[2].get<float>(), HairColor[3].get<float>());
		}
		else if (MaterialName.find("Body") != std::string::npos)
		{
			auto& BodyColor = Root["BodyOutlineColor"];
			GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor = Vector4(BodyColor[0].get<float>(), BodyColor[1].get<float>(), BodyColor[2].get<float>(), BodyColor[3].get<float>());
		}
		else
		{
			auto& DefColor = Root["DefOutlineColor"];
			GET_CONSTBUFFER(PBRConstantBuffer).OutLineColor = Vector4(DefColor[0].get<float>(), DefColor[1].get<float>(), DefColor[2].get<float>(), DefColor[3].get<float>());
		}
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

