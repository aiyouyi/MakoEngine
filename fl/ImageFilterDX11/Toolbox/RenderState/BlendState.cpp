#include "BlendState.h"

CC3DBlendState::CC3DBlendState()
{
	bAlphaToCoverageEnable = false;
	bIndependentBlendEnable = false;
	for (unsigned int i = 0; i < MAX_RENDER_TARGET_NUM; i++)
	{
		bBlendEnable[i] = false;
		ucSrcBlend[i] = BP_ONE;
		ucDestBlend[i] = BP_ZERO;
		ucBlendOp[i] = BO_ADD;

		bAlphaBlendEnable[i] = false;
		ucSrcBlendAlpha[i] = BP_ONE;
		ucDestBlendAlpha[i] = BP_ZERO;
		ucBlendOpAlpha[i] = BO_ADD;
		ucWriteMask[i] = WM_ALL;
	}
}
