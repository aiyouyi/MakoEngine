#ifndef __GLDepthStencilState__
#define  __GLDepthStencilState__

#include "RenderState/DepthStencilState.h"
#include "BaseDefine/Define.h"

class GLDepthStencilState : public CC3DDepthStencilState
{
public:
	GLDepthStencilState();
	virtual ~GLDepthStencilState();

	virtual void CreateState();

	bool IsDepthEnable() const;
	uint32_t DepthCompare() const;
};

#endif