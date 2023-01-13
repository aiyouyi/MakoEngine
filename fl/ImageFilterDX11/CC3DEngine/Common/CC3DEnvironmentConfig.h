#ifndef _CC_ENVIRONMENT_CONFIG_H_
#define _CC_ENVIRONMENT_CONFIG_H_
#include "CC3DEngine/Material/EffectConfig.h"

class MaterialTexRHI;

namespace CC3DImageFilter
{
	class CC3DSceneManage;
	class CC3DPbrRender;
}

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
	bool dynamicPbrMat = false;
	int MaterialType = 0;
	std::string resourth_path = "";

	std::mutex MapEffectLock;
	std::map<std::string, std::shared_ptr<CC3DImageFilter::EffectConfig>> MapEffectConfig;
	std::shared_ptr<CC3DImageFilter::CC3DSceneManage> SceneMgr;
	std::shared_ptr<CC3DImageFilter::CC3DPbrRender> PbrRender;
};

#endif