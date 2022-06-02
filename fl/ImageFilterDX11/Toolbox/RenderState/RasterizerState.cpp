#include "RasterizerState.h"

CC3DRasterizerState::CC3DRasterizerState()
{
	m_bWireEnable = false;
	m_uiCullType = CT_CCW;
	m_bClipPlaneEnable = false;
	m_bScissorTestEnable = false;
	m_fDepthBias = 0.0f;
}

CC3DRasterizerState::~CC3DRasterizerState()
{

}