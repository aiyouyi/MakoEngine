#include "GLIndexBuffer.h"

GLIndexBuffer::GLIndexBuffer()
{

}

GLIndexBuffer::~GLIndexBuffer()
{
	if (EBO)
	{
		GL_DELETE_BUFFER(EBO);
	}
}

void GLIndexBuffer::CreateIndexBuffer(unsigned short* pData, int nTriangle)
{
	m_IndexFormat = GL_UNSIGNED_SHORT;
	m_NumberTrangle = nTriangle;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * m_NumberTrangle * 3, pData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLIndexBuffer::CreateIndexBuffer(unsigned int* pData, int nTriangle)
{
	m_IndexFormat = GL_UNSIGNED_INT;
	m_NumberTrangle = nTriangle;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_NumberTrangle * 3, pData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
}

void GLIndexBuffer::UpdateIndexBuffer(unsigned short* pData, int nTriangle, int sizePerTrangle, int offset)
{

}

uint32_t GLIndexBuffer::GetNumberTriangle() const
{
	return m_NumberTrangle;
}

int32_t GLIndexBuffer::GetIndexFormat() const
{
	return m_IndexFormat;
}
