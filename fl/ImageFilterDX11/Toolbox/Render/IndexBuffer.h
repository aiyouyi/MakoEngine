#ifndef CC3D_INDEXBUFFER
#define CC3D_INDEXBUFFER

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DIndexBuffer
{
public:
	CC3DIndexBuffer();
	virtual ~CC3DIndexBuffer();

	virtual void CreateIndexBuffer(unsigned short* pData, int nTriangle) = 0;
	virtual void CreateIndexBuffer(unsigned int* pData, int nTriangle) = 0;
	virtual uint32_t GetNumberTriangle() const = 0;
	virtual int32_t GetIndexFormat() const = 0;
};
#endif