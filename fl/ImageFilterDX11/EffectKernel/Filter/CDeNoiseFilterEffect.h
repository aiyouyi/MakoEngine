#pragma once

#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/RectDraw.h"
#include "EffectKernel/DXBasicSample.h"

constexpr int const_record_fbo_num = 6;

class CDeNoiseFilterEffect : public CEffectPart, protected DXBaicSample
{
public:
	CDeNoiseFilterEffect();
	virtual ~CDeNoiseFilterEffect();
	virtual void Release() override;
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam &RenderParam) override;
	virtual void reset();

private:
	void GreyRender(BaseRenderParam &RenderParam);
	void kgAndResCovRender(BaseRenderParam &RenderParam);
	void DenoiseRender(BaseRenderParam &RenderParam);

private:
	bool m_bInit;

	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;

	DX11Shader *m_denoiseProgram;
	DX11Shader *m_greyProgram;
	DX11Shader *m_kgAndResCovProgram;

	ID3D11Buffer*  m_pConstantBuffer;  //×ª»»¾ØÕó

	float m_vPredictionCov;
	float m_vMeasureCov;
	float m_vFilterCov;
	float m_fWeight;

	DX11FBO *m_lastFilterRes;
	DX11FBO *m_lastFilterCovTexture;
	DX11FBO *m_recordImageArray[const_record_fbo_num];

	int m_lastIndex;
	bool m_bInitRecordImage;
};