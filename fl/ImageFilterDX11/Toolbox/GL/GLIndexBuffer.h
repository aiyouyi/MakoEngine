#ifndef __GLINDEXBUFFER__
#define __GLINDEXBUFFER__

#include "Render/IndexBuffer.h"
#include "openglutil.h"

class GLIndexBuffer : public CC3DIndexBuffer
{
public:
	GLIndexBuffer();
	virtual ~GLIndexBuffer();

	virtual void CreateIndexBuffer(unsigned short* pData, int nTriangle);
	virtual void CreateIndexBuffer(unsigned int* pData, int nTriangle);
	virtual void UpdateIndexBuffer(unsigned short* pData, int nTriangle, int sizePerTrangle, int offset);
	virtual uint32_t GetNumberTriangle() const;
	virtual int32_t GetIndexFormat() const;

	GLuint EBO = 0;
private:
	uint32_t m_NumberTrangle = 0;
	int32_t m_IndexFormat = GL_UNSIGNED_INT;
};

#endif