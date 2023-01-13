#include "ShaderRHI.h"

ShaderRHI::ShaderRHI()
{

}

ShaderRHI::~ShaderRHI()
{

}

std::shared_ptr< GLProgramBase> ShaderRHI::GetGLProgram()
{
	return std::shared_ptr< GLProgramBase>();
}
