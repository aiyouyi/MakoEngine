#pragma once
#include "Common/CC3DUtils.h"
#include "Toolbox/Render/DynamicRHI.h"

namespace CC3DImageFilter
{
	struct EffectConfig;
}

class MaterialCreator
{
public:
	static void CreateMaterial(tinygltf::Model* Model, const std::vector<std::shared_ptr<CC3DTextureRHI>>& ModelTexture,
		std::vector<class CC3DMaterial*>& ModelMaterial, CC3DImageFilter::EffectConfig* EffectConfig);
};