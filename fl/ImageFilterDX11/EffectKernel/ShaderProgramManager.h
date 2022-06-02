#ifndef _SHADER_PROGRAM_MANAGER_H_
#define _SHADER_PROGRAM_MANAGER_H_

#include <unordered_map>
#include <string>

struct CCVetexAttribute;
class DX11Shader;

class ShaderProgramManager
{
public:
	static ShaderProgramManager* GetInstance();
	~ShaderProgramManager();

	void Release();

	static void ReleaseInstance();
	/**
	 * Get or Create a Shader Program by path and input layout attribs, then put into an unordered map structure
	 *
	 * @param str_path   absolute path of the shader file(.fx)
	 * @param pAttribs   input vertex layout attribs
	 * @param nAttri     the size of input vertex layout attribs
	 */
	DX11Shader* GetOrCreateShaderByPathAndAttribs(const std::string& str_path, CCVetexAttribute* pAttribs, int nAttri, bool seperate = false);
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
	bool RemoveShader(DX11Shader* rmv_shader);

private:
	ShaderProgramManager();
	static ShaderProgramManager* _instance;
private:
	std::unordered_map<std::string, DX11Shader*> shaderMap;
};

#endif