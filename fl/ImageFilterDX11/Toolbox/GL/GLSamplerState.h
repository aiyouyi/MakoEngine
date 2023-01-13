#ifndef __GLSamplerState__
#define __GLSamplerState__

#include "RenderState/SamplerState.h"

class GLSamplerState : public CC3DSamplerState
{
public:
	GLSamplerState();
	virtual ~GLSamplerState();

	virtual void CreateState() {}
};

#endif