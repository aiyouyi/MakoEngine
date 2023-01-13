#ifndef CC3D_VERTEXBUFFER
#define CC3D_VERTEXBUFFER

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DVertexBuffer
{
public:
	CC3DVertexBuffer();
	virtual ~CC3DVertexBuffer();

	//virtual void CreateVertexBuffer(float* pData, int32_t nVertex, int32_t nStride) = 0;
	virtual void CreateVertexBuffer(float* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex = 0) {}
	virtual void CreateVertexBuffer(uint16_t* pData, int32_t nVertex, int32_t nStride, int32_t bindIndex = 0) {}
	virtual void CreateVertexBuffer(void* pData, int StrideByteWidth, int Count,bool StreamOut) {}
	virtual void UpdateVertexBUffer(float* pData, int nVertex, int perUpdateSize, int sizePerVertex, int begin = 0, int offset = 0) = 0;
	virtual int32_t GetStride() const = 0;
	virtual int32_t GetCount() const = 0;
};


#endif