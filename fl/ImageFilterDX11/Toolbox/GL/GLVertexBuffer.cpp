#include "GLVertexBuffer.h"
#include "ToolBox/GL/openglutil.h"

GLVertexBuffer::GLVertexBuffer()
{
	for (int i=0;i<10;i++)
	{
		m_VBO[i] = 0;
	}
}


GLVertexBuffer::~GLVertexBuffer()
{
	if (VAO)
	{
		GL_DELETE_VAO(VAO);
	}
	for (int i=0;i<10;i++)
	{
		if (m_VBO[i])
		{
			GL_DELETE_BUFFER(m_VBO[i]);
		}
	}
}

void GLVertexBuffer::CreateVertexBuffer(float* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex)
{
	m_Vertex = nVertex;
	m_Stride = nStride * sizeof(float);
	InnerCreateVertexBuffer(pData, nVertex, nStride, bindIndex);
}

void GLVertexBuffer::CreateVertexBuffer(uint16_t* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex)
{
	m_Vertex = nVertex;
	m_Stride = nStride * sizeof(uint16_t);
	InnerCreateVertexBuffer(pData, nVertex, nStride, bindIndex);
}

void GLVertexBuffer::CreateVertexBuffer(float* pVertices, int nVerts, int* pArrange, int nArrange, int nStep)
{
	m_Vertex = nVerts;
	m_Stride = nStep * sizeof(float);
	OpenGLUtil::createVAO(VAO, m_VBO[0], pVertices, nVerts, pArrange, nArrange, nStep);
}

void GLVertexBuffer::UpdateVertexBUffer(float* pData, int nVertex, int perUpdateSize, int sizePerVertex, int begin /*= 0*/, int offset /*= 0*/)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[begin]);
	GL_CHECK();
	glBufferData(GL_ARRAY_BUFFER, sizePerVertex * nVertex, pData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLVertexBuffer::InitBufferCount(int32_t bufferCount)
{
	m_VBOCount = bufferCount;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(m_VBOCount, m_VBO);
	GL_CHECK();
	m_Init = true;
}

int32_t GLVertexBuffer::GetStride() const
{
	return m_Stride;
}

int32_t GLVertexBuffer::GetCount() const
{
	return m_Vertex;
}
