#include "DepthStencilState.h"

CC3DDepthStencilState::CC3DDepthStencilState()
{
	m_bDepthEnable = true;
	m_bDepthWritable = true;
	m_uiDepthCompareMethod = CM_LESSEQUAL;


	m_bStencilEnable = false;
	m_uiStencilCompareMethod = CM_ALWAYS;
	m_uiReference = 0x0;
	m_uiMask = 0xFF;
	m_uiWriteMask = 0xFF;
	m_uiSPassZPassOP = OT_KEEP;
	m_uiSPassZFailOP = OT_KEEP;
	m_uiSFailZPassOP = OT_KEEP;

	m_uiCCW_StencilCompareMethod = CM_ALWAYS;
	m_uiCCW_SPassZPassOP = OT_KEEP;
	m_uiCCW_SPassZFailOP = OT_KEEP;
	m_uiCCW_SFailZPassOP = OT_KEEP;
	m_bTwoSideStencilMode = false;
}

CC3DDepthStencilState::~CC3DDepthStencilState()
{

}
