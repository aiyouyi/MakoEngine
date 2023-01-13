#ifndef __GLConstantBuffer__
#define __GLConstantBuffer__

#include "Render/ConstantBuffer.h"

class GLConstantBuffer : public CC3DConstantBuffer
{
public:
	GLConstantBuffer();
	virtual ~GLConstantBuffer();

	virtual void CreateBuffer(int32_t ByteSize) {};
};

#endif