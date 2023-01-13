#ifndef __GLBaseTypeTraits__
#define __GLBaseTypeTraits__

#include "BaseDefine/Define.h"

template<typename T>
struct TGLBaseTypeTraits
{
};

template<>
struct TGLBaseTypeTraits<float>
{
	static const int Type = GL_FLOAT;
};

template<>
struct TGLBaseTypeTraits<uint16_t>
{
	static const int Type = GL_UNSIGNED_SHORT;
};

#endif