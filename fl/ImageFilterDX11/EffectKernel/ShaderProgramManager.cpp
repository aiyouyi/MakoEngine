#include "ShaderProgramManager.h"
#include <mutex>
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DXUtils.h"

ShaderProgramManager* ShaderProgramManager::_instance = nullptr;

std::recursive_mutex g_mutex;

ShaderProgramManager* ShaderProgramManager::GetInstance()
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	if (_instance == nullptr)
	{
		_instance = new ShaderProgramManager();
		return _instance;
	}

	return _instance;
}

ShaderProgramManager::~ShaderProgramManager()
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	auto itr = shaderMap.begin();
	for (; itr != shaderMap.end(); )
	{
		if (itr->second)
			SAFE_DELETE(itr->second);
		shaderMap.erase(itr++);
	}
}

void ShaderProgramManager::Release()
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	auto itr = shaderMap.begin();
	for (; itr != shaderMap.end(); )
	{
		if (itr->second)
			SAFE_DELETE(itr->second);
		shaderMap.erase(itr++);
	}
}

void ShaderProgramManager::ReleaseInstance()
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	SAFE_DELETE(_instance);
}

DX11Shader* ShaderProgramManager::GetOrCreateShaderByPathAndAttribs(const std::string& str_path, CCVetexAttribute* pAttribs, int nAttri, bool separate /* = false */)
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	if (str_path.empty())
		return nullptr;

	auto sm_itr = shaderMap.find(str_path);
	if (sm_itr != shaderMap.end())
	{
		return sm_itr->second;
	}
	else
	{
		DX11Shader* pShader = new DX11Shader();
		pShader->SetSeparate(separate);
		DXUtils::CompileShaderWithFile(pShader, (char*)str_path.c_str(), pAttribs, nAttri);
		shaderMap[str_path] = pShader;
		return pShader;
	}
	return nullptr;
}

bool ShaderProgramManager::RemoveShader(const std::string& str_path)
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	bool ret_value = false;
	auto itr = shaderMap.begin();
	for (; itr != shaderMap.end(); )
	{
		if (itr->first == str_path)
		{
			SAFE_DELETE(itr->second);
			shaderMap.erase(itr++);
			ret_value = true;
			break;
		}
		else
		{
			++itr;
		}
	}

	return ret_value;
}

bool ShaderProgramManager::RemoveShader(DX11Shader* rmv_shader)
{
	std::lock_guard< std::recursive_mutex> lock(g_mutex);
	bool ret_value = false;
	auto itr = shaderMap.begin();
	for (; itr != shaderMap.end(); )
	{
		if (itr->second == rmv_shader)
		{
			SAFE_DELETE(itr->second);
			shaderMap.erase(itr++);
			ret_value = true;
			break;
		}
		else
		{
			++itr;
		}
	}

	return ret_value;
}

ShaderProgramManager::ShaderProgramManager()
{

}