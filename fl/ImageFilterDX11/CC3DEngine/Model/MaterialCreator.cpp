#include "MaterialCreator.h"
#include "Material/CC3DPbrMaterial.h"
#include "Material/CC3DFurMaterial.h"
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "Toolbox/Render/TextureRHI.h"


void MaterialCreator::CreateMaterial(tinygltf::Model* Model, const std::vector<std::shared_ptr<CC3DTextureRHI>>& ModelTexture,
	std::vector<class CC3DMaterial*>& ModelMaterial, CC3DImageFilter::EffectConfig* EffectConfig)
{
	for (int i = 0; i < Model->materials.size(); i++)
	{

		if (Model->materials[i].name == EffectConfig->FurData.ConfigData.FurMaterialName)
		{
			CC3DFurMaterial* pMaterial = new CC3DFurMaterial();
			pMaterial->initModel(Model);
			pMaterial->InitMaterial(i, ModelTexture);
			pMaterial->alphaMode = "BLEND";
			pMaterial->InitShaderProgram(CC3DEnvironmentConfig::getInstance()->resourth_path, EffectConfig);
			ModelMaterial.push_back(pMaterial);
		}
		else
		{
			auto& MaterialName = Model->materials[i].name;
			CC3DPBRMaterial* pMaterial = new CC3DPBRMaterial();
			pMaterial->initModel(Model);
			pMaterial->InitMaterial(i, ModelTexture);
			pMaterial->InitShaderProgram(CC3DEnvironmentConfig::getInstance()->resourth_path, EffectConfig);

			ModelMaterial.push_back(pMaterial);
		}

	}
	if (ModelMaterial.size() == 0)
	{
		CC3DPBRMaterial* pMaterial = new CC3DPBRMaterial();
		pMaterial->initModel(Model);
		pMaterial->CreateDefault();
		pMaterial->InitShaderProgram("", EffectConfig);
		ModelMaterial.push_back(pMaterial);
	}
}
