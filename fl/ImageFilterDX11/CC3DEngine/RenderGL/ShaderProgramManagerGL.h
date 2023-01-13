#ifndef _SHADER_PROGRAM_MANAGER_H_
#define _SHADER_PROGRAM_MANAGER_H_

#include <unordered_map>
#include <string>

#include "ToolBox/GL/CCProgram.h"

class ShaderProgramManagerGL
{
public:
	static ShaderProgramManagerGL* GetInstance();
	~ShaderProgramManagerGL();

	void Release();

	static void ReleaseInstance();
	/**
	 * Get or Create a Shader Program by path and input layout attribs, then put into an unordered map structure
	 *
	 * @param str_path   absolute path of the shader file(.fx)
	 * @param pAttribs   input vertex layout attribs
	 * @param nAttri     the size of input vertex layout attribs
	 */
	CCProgram* GetOrCreateShaderByPath(const std::string& vs_path, const std::string& fs_path);
	/**
	 * Remove a Shader Program by path
	 *
	 * @param str_path   absolute path of the shader file(.fx)
	*/
	bool RemoveShader(const std::string& str_path);
	/**
	 * Remove a Shader Program by shader pointer
	 *
	 * @param rmv_shader   the pointer to the shader
	*/
	//bool RemoveShader(DX11Shader* rmv_shader);

private:
	ShaderProgramManagerGL();
	static ShaderProgramManagerGL* _instance;
private:
	std::unordered_map<std::string, CCProgram*> shaderMap;
};

#endif