#include "GetMaterialData.h"
#include "Material/CC3DMaterial.h"

std::string GetMaterialData::GetAlphaMode(CC3DMaterial* Material)
{
	return Material->alphaMode;
}
