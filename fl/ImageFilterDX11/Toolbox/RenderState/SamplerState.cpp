#include "SamplerState.h"

CC3DSamplerState::CC3DSamplerState()
{
	m_uiMag = FM_POINT;
	m_uiMin = FM_POINT;
	m_uiMip = FM_NONE;
	m_uiMipLevel = 0;
	m_uiAniLevel = 1;

	m_uiCoordU = CM_WRAP;
	m_uiCoordV = CM_WRAP;
	m_uiCoordW = CM_WRAP;
	m_BorderColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_MipLODBias = 0.0f;
}

CC3DSamplerState::~CC3DSamplerState()
{

}

