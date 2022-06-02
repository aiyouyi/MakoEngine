#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Drawable.hpp"
//#include "EffectModel.hpp"
#include "Toolbox/DXUtils/DXUtils.h"
#include <vector>
class CFilterWithMask :public CEffectPart
{
public:
	CFilterWithMask();
	~CFilterWithMask();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

private:
	ID3D11Buffer*       m_pConstantBufferMask;  
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;

};

