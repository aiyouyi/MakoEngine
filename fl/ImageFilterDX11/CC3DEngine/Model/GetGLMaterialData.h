#pragma once
#include <string>

class CC3DMaterialGL;

class GetGLMaterialData
{
public:
	static std::string GetAlphaMode(CC3DMaterialGL* Material);
};