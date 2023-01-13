#pragma once

#include "EffectKernel/CEffectPart.h"
#include "Toolbox/EffectModel.hpp"
#include "Toolbox/FSObject.h"
#include "Toolbox/FacialObject.h"
#include "EffectKernel/DXBasicSample.h"

class CFaceEffect3DModel : public CEffectPart, protected DXBaicSample
{
public:
	CFaceEffect3DModel();
	~CFaceEffect3DModel();

	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

private:
	bool loadFromXML(HZIP hZip, const char *szFilePath, const char *szFileXml);

	vector<EffectModel> m_vEffectModel;

	__int64 m_nStartTime;
	__int64 m_nLoopStartTime;
	__int64 m_nLastTime;
	long m_nEffectDuring;
	long m_nAniLoopTime;

	//��Ⱦģ����Ϣ
	DX11Shader *m_pShader;
	ID3D11Buffer*       m_pConstantBuffer;  //ת������

	//��Ⱦ��ͷ��ģ��
	DX11Shader *m_pShaderForHeaderCull;
	ID3D11Buffer *m_pConstantBufferForHeaderCull;  //ת������

	//����״̬
	ID3D11SamplerState* m_pSamplerLinear;

	ID3D11BlendState *m_pBSEnable;
	ID3D11BlendState *m_pBSWriteDisable;
	ID3D11BlendState *m_pBSDisable;
	ID3D11BlendState *m_pBSDisableWriteDisable;

	//���Ȳ���
	ID3D11DepthStencilState *m_pDepthStateEnable;
	ID3D11DepthStencilState *m_pDepthStateDisable;
	ID3D11DepthStencilState *m_pDepthStateEnableWriteDisable;

	HeaderModel m_HeaderModel;

	bool m_hasMatcap = false;
	DX11Texture *m_MatCap = NULL;
	DX11Texture *m_MatCapNormal = NULL;

	float m_GradientTime[3] = { 0,0,0 };

};
