#include "GetGLMaterialData.h"
#include "MaterialGL/CC3DMaterialGL.h"

std::string GetGLMaterialData::GetAlphaMode(CC3DMaterialGL* Material)
{
	return Material->alphaMode;
}
