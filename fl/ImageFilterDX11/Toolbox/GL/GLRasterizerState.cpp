#include "GLRasterizerState.h"

uint32_t glCullType[] = {
			0,
		GL_BACK,
		GL_FRONT,
};

GLRasterizerState::GLRasterizerState()
{

}

GLRasterizerState::~GLRasterizerState()
{

}

void GLRasterizerState::CreateState()
{

}

bool GLRasterizerState::IsEnableCullFace() const
{
	return this->m_uiCullType != CT_NONE;
}

uint32_t GLRasterizerState::CullMode() const
{
	return glCullType[this->m_uiCullType];
}

uint32_t GLRasterizerState::CullMode(unsigned char value) const
{
	return glCullType[value];
}
