#ifndef __GLBlendState__
#define __GLBlendState__

#include "RenderState/BlendState.h"
#include "BaseDefine/Define.h"

class GLBlendState : public CC3DBlendState
{
public:
	GLBlendState();
	virtual ~GLBlendState();

	virtual void CreateState();

	bool IsBlendEnable ()const;
	uint32_t Sfactor() const;
	uint32_t Dfactor() const;
};

#endif
