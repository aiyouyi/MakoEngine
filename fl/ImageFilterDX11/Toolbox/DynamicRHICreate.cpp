
#include "Toolbox/DXUtils/DX11DynamicRHI.h"

#ifdef _WIN64
CC3DDynamicRHI* GetDynamicRHI()
{
	return DX11DynamicRHI::GetInstance();
}
#else

#include "Toolbox/GL/GLDynamicRHI.h"

CC3DDynamicRHI::RnederAPI gRenderAPI = CC3DDynamicRHI::DX11;

CC3DDynamicRHI* GetDynamicRHI()
{
	if (gRenderAPI == CC3DDynamicRHI::DX11)
	{
		DX11DynamicRHI::GetInstance()->API = CC3DDynamicRHI::DX11;
		return DX11DynamicRHI::GetInstance();
	}
	else
	{
		GLDynamicRHI::GetInstance()->API = CC3DDynamicRHI::OPENGL;
		return GLDynamicRHI::GetInstance();
	}
}
#endif