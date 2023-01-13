#include "GLMaterialCreator.h"
#include "MaterialGL/CC3DMaterialGL.h"
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include "Material/CC3DFurMaterial.h"
#include "MaterialGL/CC3DEyeMaterial.h"
#include "Material/CC3DPbrMaterial.h"

void GLMaterialCreate::CreateMaterial(tinygltf::Model* Model, const std::vector<std::shared_ptr<CC3DTextureRHI>>& ModelTexture,
	std::vector<class CC3DMaterial*>& ModelMaterial, CC3DImageFilter::EffectConfig* EffectConfig)
{
	for (int i = 0; i < Model->materials.size(); i++)
	{
		if (Model->materials[i].name == EffectConfig->FurData.ConfigData.FurMaterialName)
		{
			CC3DMaterial* pMaterial = nullptr;
			pMaterial = new CC3DFurMaterial();
			pMaterial->initModel(Model);
			pMaterial->InitMaterial(i, ModelTexture);
			pMaterial->alphaMode = "BLEND";
			pMaterial->InitShaderProgram(CC3DEnvironmentConfig::getInstance()->resourth_path, EffectConfig);

			ModelMaterial.push_back(pMaterial);
		}

		else if (Model->materials[i].name == "female_eye_color2")
		{
			CC3DMaterial* pMaterial = new CC3DEyeMaterial();
			pMaterial->initModel(Model);
			pMaterial->InitMaterial(i, ModelTexture);
			pMaterial->InitShaderProgram(CC3DEnvironmentConfig::getInstance()->resourth_path, EffectConfig);

			ModelMaterial.push_back(pMaterial);
		}
		else
		{
			CC3DMaterial* pMaterial = new CC3DPBRMaterial();

			pMaterial->InitShaderProgram(CC3DEnvironmentConfig::getInstance()->resourth_path, EffectConfig);
			pMaterial->initModel(Model);
			pMaterial->InitMaterial(i, ModelTexture);
			ModelMaterial.push_back(pMaterial);
		}
	}
	if (ModelMaterial.size() == 0)
	{
		CC3DMaterial* pMaterial = new CC3DPBRMaterial();
		//pMaterial->alphaMode = "BLEND";
		pMaterial->initModel(Model);
		pMaterial->CreateDefault();
		pMaterial->InitShaderProgram(CC3DEnvironmentConfig::getInstance()->resourth_path, EffectConfig);
		ModelMaterial.push_back(pMaterial);
	}

	CC3DEnvironmentConfig::getInstance()->dynamicPbrMat = false;
}
