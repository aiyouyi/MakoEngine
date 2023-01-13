#pragma once
#include "Toolbox/inc.h"
#include "BaseDefine/Vectors.h"
#include "CC3DEngine/Material/FurConfig.h"
#include "CC3DEngine/CC3DConfigStream.h"

namespace CC3DImageFilter
{
	struct EffectConfig
	{
		std::map < std::string, std::shared_ptr<MaterialTexRHI>> EmissiveMaskTex;

		std::shared_ptr<MaterialTexRHI> HairShiftTex ;
		std::shared_ptr<MaterialTexRHI> HairOutLineMaskTex ;
		std::shared_ptr<MaterialTexRHI> DefOutLineMaskTex ;

		CC3DImageFilter::FurConfig FurData;
		CC3DImageFilter::ModelConfig ModelConfig;
	};
}