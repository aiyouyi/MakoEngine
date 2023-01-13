#pragma once
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/TextureRHI.h"

class DX11DoubleBufferWrapper
{
public:
	static void InitDoubleBuffer( std::shared_ptr<DoubleBufferRHI> doubleBuffer,  int nWidth, int nHeight, bool UseDepth = false, bool bMSAA = false, int32_t format = CC3DTextureRHI::SFT_A8R8G8B8);
};