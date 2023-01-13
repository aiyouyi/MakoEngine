#include "GLDepthStencilState.h"

uint32_t glDepthCompare[] = {
		GL_NEVER,
		GL_LESS,
		GL_EQUAL,
		GL_LEQUAL,
		GL_GREATER,
		GL_NOTEQUAL,
		GL_GEQUAL,
		GL_ALWAYS,
};

GLDepthStencilState::GLDepthStencilState()
{

}

GLDepthStencilState::~GLDepthStencilState()
{

}

void GLDepthStencilState::CreateState()
{
	
}

bool GLDepthStencilState::IsDepthEnable() const
{
	return m_bDepthEnable;
}

uint32_t GLDepthStencilState::DepthCompare() const
{
	return glDepthCompare[this->m_uiDepthCompareMethod];
}
