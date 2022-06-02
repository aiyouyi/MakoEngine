#ifndef BLEND_STATE
#define BLEND_STATE

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DBlendState
{
public:
	enum
	{
		MAX_RENDER_TARGET_NUM = 8
	};
	enum //Blend Parameter
	{
		BP_ZERO,
		BP_ONE,
		BP_SRCCOLOR,
		BP_INVSRCCOLOR,
		BP_SRCALPHA,
		BP_INVSRCALPHA,
		BP_DESTALPHA,
		BP_INVDESTALPHA,
		BP_DESTCOLOR,
		BP_INVDESTCOLOR,
		BP_MAX
	};
	enum// Blend OP
	{
		BO_ADD,
		BO_SUBTRACT,
		BO_REVSUBTRACT,
		BO_MIN_SRC_DEST,
		BO_MAX_SRC_DEST,
		BO_MAX

	};
	enum //Write Mask
	{
		WM_NONE = 0,
		WM_ALPHA = BIT(0),
		WM_RED = BIT(1),
		WM_Green = BIT(2),
		WM_BLUE = BIT(3),
		WM_ALL = 0X0F
	};
	CC3DBlendState();
	virtual ~CC3DBlendState()
	{

	}

	virtual void CreateState() = 0;

	bool			bAlphaToCoverageEnable;
	bool			bIndependentBlendEnable;

	bool			bBlendEnable[MAX_RENDER_TARGET_NUM];
	unsigned char   ucSrcBlend[MAX_RENDER_TARGET_NUM];
	unsigned char   ucDestBlend[MAX_RENDER_TARGET_NUM];
	unsigned char	ucBlendOp[MAX_RENDER_TARGET_NUM];

	bool			bAlphaBlendEnable[MAX_RENDER_TARGET_NUM];
	unsigned char   ucSrcBlendAlpha[MAX_RENDER_TARGET_NUM];
	unsigned char	ucDestBlendAlpha[MAX_RENDER_TARGET_NUM];
	unsigned char	ucBlendOpAlpha[MAX_RENDER_TARGET_NUM];
	unsigned char   ucWriteMask[MAX_RENDER_TARGET_NUM];
};

#endif