#ifndef CC3D_SamplerState
#define CC3D_SamplerState

#include "BaseDefine/Vectors.h"

class CC3DSamplerState
{
public:
	enum //Coordinate Mode
	{
		CM_CLAMP,
		CM_WRAP,
		CM_MIRROR,
		CM_BORDER,
		CM_MAX
	};
	enum //Filter Method
	{
		FM_NONE,
		FM_POINT,
		FM_LINE,
		FM_ANISOTROPIC,
		FM_MAX

	};

	CC3DSamplerState();
	virtual ~CC3DSamplerState();

	virtual void CreateState() = 0;

public:
	unsigned char m_uiMag;
	unsigned char m_uiMin;
	unsigned char m_uiMip;
	unsigned char m_uiMipLevel;
	unsigned char m_uiAniLevel;

	unsigned char m_uiCoordU;
	unsigned char m_uiCoordV;
	unsigned char m_uiCoordW;
	float        m_MipLODBias;
	Vector4  m_BorderColor;
};

#endif