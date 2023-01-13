#include "DXBasicSample.h"
#include "Toolbox/DXUtils/DXUtils.h"

DXBaicSample::DXBaicSample()
{
	m_pSamplerLinear = DXUtils::SetSampler();
	m_pBlendStateNormal = DXUtils::CreateBlendState();
}

DXBaicSample::~DXBaicSample()
{
	SAFE_RELEASE_BUFFER(m_pSamplerLinear);
	SAFE_RELEASE_BUFFER(m_pBlendStateNormal);
}

