#include "CC3DEnvironmentConfig.h"
#include "BaseDefine/Define.h"
#include <mutex>

std::mutex g_mutex;
CC3DEnvironmentConfig* CC3DEnvironmentConfig::_instance = nullptr;

CC3DEnvironmentConfig* CC3DEnvironmentConfig::getInstance()
{
	if (_instance == nullptr)
	{
		g_mutex.lock();
		if (_instance == nullptr)
		{
			_instance = new CC3DEnvironmentConfig();
			g_mutex.unlock();
			return _instance;
		}
		g_mutex.unlock();
	}

	return _instance;
}

void CC3DEnvironmentConfig::Release()
{
	SAFE_DELETE(_instance);
}

CC3DEnvironmentConfig::~CC3DEnvironmentConfig()
{

}

CC3DEnvironmentConfig::CC3DEnvironmentConfig()
{

}

