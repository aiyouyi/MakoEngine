#pragma once
#ifndef CFaceLevel_H
#define CFaceLevel_H

#include "EffectKernel/CEffectPart.h"
#include "EffectKernel/DXBasicSample.h"

class CFaceLevel : public CEffectPart ,protected DXBaicSample
{
public:
	CFaceLevel();
	virtual ~CFaceLevel();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);
private:
	int m_nWidth;
	int m_nHeight;

	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
	ID3D11Buffer* m_pConstantBuffer;
};

#endif // CFaceLevel_H
