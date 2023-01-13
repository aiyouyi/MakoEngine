#ifndef __GLVertexBuffer__
#define __GLVertexBuffer__

#include "Render/VertexBuffer.h"
#include "GLBaseTypeTraits.h"

class GLVertexBuffer : public CC3DVertexBuffer
{
public:
	GLVertexBuffer();
	virtual ~GLVertexBuffer();


	virtual void CreateVertexBuffer(float* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex = 0);
	virtual void CreateVertexBuffer(uint16_t* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex = 0);
	virtual void CreateVertexBuffer(float* pVertices, int nVerts, int* pArrange, int nArrange, int nStep);
	virtual void UpdateVertexBUffer(float* pData, int nVertex, int perUpdateSize, int sizePerVertex, int begin = 0, int offset = 0);
	virtual void InitBufferCount(int32_t bufferCount);
	virtual int32_t GetStride() const;
	virtual int32_t GetCount() const ;

private:
	template<typename T>
	void InnerCreateVertexBuffer(T* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex)
	{
		if (!m_Init)
		{
			glGenVertexArrays(1, &VAO);
			glGenBuffers(m_VBOCount, m_VBO);
			m_Init = true;
		}

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[bindIndex]);
		glBufferData(GL_ARRAY_BUFFER, nVertex * nStride * sizeof(T), pData, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(bindIndex);
		glVertexAttribPointer(bindIndex, nStride , TGLBaseTypeTraits<T>::Type, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

public:
	GLuint VAO = 0;
private:
	GLuint m_VBO[10] = {};
	int m_VBOCount = 1;
	int32_t m_Stride = 0;
	int32_t m_Vertex = 0;
	bool m_Init = false;
};

#endif
