#ifndef CC3D_ConstantBuffer
#define CC3D_ConstantBuffer

#include "Toolbox/inc.h"

class CC3DConstantBuffer
{
public:
	CC3DConstantBuffer();
	virtual ~CC3DConstantBuffer();

	virtual void CreateBuffer(int32_t ByteSize) = 0;
};

#endif