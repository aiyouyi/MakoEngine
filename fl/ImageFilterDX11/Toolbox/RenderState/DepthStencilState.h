#ifndef CC3D_DepthStencilState
#define CC3D_DepthStencilState

class CC3DDepthStencilState
{
public:
	enum //Compare Nethod
	{
		CM_NEVER,
		CM_LESS,
		CM_EQUAL,
		CM_LESSEQUAL,
		CM_GREATER,
		CM_NOTEQUAL,
		CM_GREATEREQUAL,
		CM_ALWAYS,
		CM_MAX

	};
	enum //OperationType
	{
		OT_KEEP,
		OT_ZERO,
		OT_REPLACE,
		OT_INCREMENT,
		OT_DECREMENT,
		OT_INVERT,
		OT_INCREMENT_STA,
		OT_DECREMENT_STA,
		OT_MAX
	};

	CC3DDepthStencilState();
	virtual ~CC3DDepthStencilState();

	virtual void CreateState() = 0;

	bool m_bDepthEnable;
	bool m_bDepthWritable;
	unsigned char m_uiDepthCompareMethod;


	bool m_bStencilEnable;
	unsigned char m_uiStencilCompareMethod;
	unsigned char m_uiReference;
	unsigned char m_uiMask;
	unsigned char m_uiWriteMask;
	unsigned char m_uiSPassZPassOP;
	unsigned char m_uiSPassZFailOP;
	unsigned char m_uiSFailZPassOP;

	unsigned char m_uiCCW_StencilCompareMethod;
	unsigned char m_uiCCW_SPassZPassOP;
	unsigned char m_uiCCW_SPassZFailOP;
	unsigned char m_uiCCW_SFailZPassOP;
	bool m_bTwoSideStencilMode;
};

#endif