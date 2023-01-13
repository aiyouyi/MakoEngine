#ifndef __GLRasterizerState__
#define __GLRasterizerState__

#include "RenderState/RasterizerState.h"
#include "BaseDefine/Define.h"

class GLRasterizerState : public CC3DRasterizerState
{
public:
	GLRasterizerState();
	virtual ~GLRasterizerState();

	virtual void CreateState();

	bool IsEnableCullFace() const;
	uint32_t CullMode() const;
	uint32_t CullMode(unsigned char value) const;
};

#endif