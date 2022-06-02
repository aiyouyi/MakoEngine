#include "ShaderProgramManager.h"
#include <mutex>
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DXUtils.h"

std::mutex g_shader_mutex;
ShaderProgramManager* ShaderProgramManager::_instance = nullptr;

ShaderProgramManager* ShaderProgramManager::GetInstance()
{
	if (_instance == nullptr)
	{
		g_shader_mutex.lock();
		if (_instance == nullptr)
		{
			_instance = new ShaderProgramManager();
			g_shader_mutex.unlock();
			return _instance;
		}
		g_shader_mutex.unlock();
	}

	return _instance;
}

ShaderProgramManager::~ShaderProgramManager()
{
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
	SAFE_DELETE(_instance);
}

DX11Shader* ShaderProgramManager::GetOrCreateShaderByPathAndAttribs(const std::string& str_path, CCVetexAttribute* pAttribs, int nAttri, bool separate /* = false */)
{
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
		pShader->m_Separate = separate;
		DXUtils::CompileShaderWithFile(pShader, (char*)str_path.c_str(), pAttribs, nAttri);
		shaderMap[str_path] = pShader;
		return pShader;
	}
	return nullptr;
}

bool ShaderProgramManager::RemoveShader(const std::string& str_path)
{
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