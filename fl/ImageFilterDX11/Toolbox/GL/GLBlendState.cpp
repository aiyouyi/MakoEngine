#include "GLBlendState.h"

GLenum glBlendState[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
};

GLBlendState::GLBlendState()
{

}

GLBlendState::~GLBlendState()
{

}

void GLBlendState::CreateState()
{
}

bool GLBlendState::IsBlendEnable() const
{
	return bBlendEnable[0];
}

uint32_t GLBlendState::Sfactor() const
{
	return glBlendState[ucSrcBlend[0]];
}

uint32_t GLBlendState::Dfactor() const
{
	return glBlendState[ucDestBlend[0]];
}
