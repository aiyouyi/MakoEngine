#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Drawable.hpp"
//#include "EffectModel.hpp"
#include "Toolbox/DXUtils/DXUtils.h"
#include <vector>
class CBGDraw :public CEffectPart
{
public:
	CBGDraw();
	~CBGDraw();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

private:

	Drawable *m_2DBGAnimation;
	ID3D11Buffer*       m_pConstantBufferMask;  
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;

	__int64 m_nStartTime;
	__int64 m_nLoopStartTime;
	__int64 m_nLastTime;
	long m_nEffectDuring;
	long m_nAniLoopTime;
};

