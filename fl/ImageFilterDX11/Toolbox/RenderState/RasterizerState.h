#ifndef CC3D_RasterizerState
#define CC3D_RasterizerState

class CC3DRasterizerState
{
public:
	enum //Cull Type
	{
		CT_NONE,
		CT_CW,
		CT_CCW,
		CT_MAX

	};

public:
	CC3DRasterizerState();
	virtual ~CC3DRasterizerState();
	
	virtual void CreateState() = 0;

	bool m_bWireEnable;
	unsigned char m_uiCullType;
	bool m_bClipPlaneEnable;
	bool m_bScissorTestEnable;
	float	m_fDepthBias;
};

#endif