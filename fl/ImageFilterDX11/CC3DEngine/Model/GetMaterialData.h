#pragma once
#include <string>

class CC3DMaterial;

class GetMaterialData
{
public:
	static std::string GetAlphaMode(CC3DMaterial* Material);
};