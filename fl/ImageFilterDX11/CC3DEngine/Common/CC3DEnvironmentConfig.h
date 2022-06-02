#ifndef _CC_ENVIRONMENT_CONFIG_H_
#define _CC_ENVIRONMENT_CONFIG_H_
#include <vector>
#include <string>
#include "BaseDefine/Vectors.h"


class CC3DEnvironmentConfig
{
public:
	static CC3DEnvironmentConfig* getInstance();
	~CC3DEnvironmentConfig();
	static void Release();
private:
	CC3DEnvironmentConfig();
	static CC3DEnvironmentConfig* _instance;
public:
	float deltaTime = 0.1f;
	std::string fur_material_name;

	std::string resourth_path="";
};

#endif