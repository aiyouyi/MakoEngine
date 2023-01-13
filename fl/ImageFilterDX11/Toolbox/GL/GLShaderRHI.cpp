#include "GLShaderRHI.h"
#include "Toolbox/GL/CCProgram.h"

GLShaderRHI::GLShaderRHI()
{
	Shader = std::make_shared<CCProgram>();
}

GLShaderRHI::~GLShaderRHI()
{

}


bool GLShaderRHI::InitShader(const std::string& vs_path, const std::string& ps_path)
{
	Shader->CreatePorgramForFile(vs_path.c_str(), ps_path.c_str());
	return Shader->GetProgramID() != 0;
}

void GLShaderRHI::UseShader()
{
	if (Shader)
	{
		Shader->Use();
	}
}

std::shared_ptr< GLProgramBase> GLShaderRHI::GetGLProgram()
{
	return Shader;
}

