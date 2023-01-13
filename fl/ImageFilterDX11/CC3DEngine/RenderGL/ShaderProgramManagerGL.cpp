#include "ShaderProgramManagerGL.h"
#include <mutex>

std::mutex g_shader_mutex;
ShaderProgramManagerGL* ShaderProgramManagerGL::_instance = nullptr;

ShaderProgramManagerGL* ShaderProgramManagerGL::GetInstance()
{
	if (_instance == nullptr)
	{
		g_shader_mutex.lock();
		if (_instance == nullptr)
		{
			_instance = new ShaderProgramManagerGL();
			g_shader_mutex.unlock();
			return _instance;
		}
		g_shader_mutex.unlock();
	}

	return _instance;
}

ShaderProgramManagerGL::~ShaderProgramManagerGL()
{
	auto itr = shaderMap.begin();
	for (; itr != shaderMap.end(); )
	{
		if (itr->second)
			SAFE_DELETE(itr->second);
		shaderMap.erase(itr++);
	}
}

void ShaderProgramManagerGL::Release()
{
	auto itr = shaderMap.begin();
	for (; itr != shaderMap.end(); )
	{
		if (itr->second)
			SAFE_DELETE(itr->second);
		shaderMap.erase(itr++);
	}
}

void ShaderProgramManagerGL::ReleaseInstance()
{
	SAFE_DELETE(_instance);
}

CCProgram* ShaderProgramManagerGL::GetOrCreateShaderByPath(const std::string& vs_path, const std::string& fs_path)
{
	if (vs_path.empty() || fs_path.empty())
		return nullptr;

	auto sm_itr = shaderMap.find(fs_path);
	if (sm_itr != shaderMap.end())
	{
		return sm_itr->second;
	}
	else
	{
		CCProgram* pShader = new CCProgram();
		pShader->CreatePorgramForFile(vs_path.c_str(), fs_path.c_str());
		shaderMap[fs_path] = pShader;
		return pShader;
	}
	return nullptr;
}

bool ShaderProgramManagerGL::RemoveShader(const std::string& str_path)
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

//bool ShaderProgramManager::RemoveShader(DX11Shader* rmv_shader)
//{
//	bool ret_value = false;
//	auto itr = shaderMap.begin();
//	for (; itr != shaderMap.end(); )
//	{
//		if (itr->second == rmv_shader)
//		{
//			SAFE_DELETE(itr->second);
//			shaderMap.erase(itr++);
//			ret_value = true;
//			break;
//		}
//		else
//		{
//			++itr;
//		}
//	}
//
//	return ret_value;
//}

ShaderProgramManagerGL::ShaderProgramManagerGL()
{

}