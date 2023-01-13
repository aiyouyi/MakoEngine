#pragma once
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include "Toolbox/RectDraw.h"
#include "Toolbox/mathlib.h"
#include "Toolbox/EffectModel.hpp"
#include "Toolbox/xmlParser.h"
#include "Toolbox/zip/unzip.h"
#include "StickerEffect.h"

class CC3DTextureRHI;

class SLinearData
{
public:
	SLinearData();

	bool load(const char *szAction);
	bool loadFromBuffer(char *pDataBuffer, int nBufferSize);

	void load(vector<float> &arrTime, vector<float> &arrValue, int nKeys, int fps);

	bool eval(float fTime, vec4 &vTrans, int &nDivideR, int &nDivideC);

	void destory();
private:
	int *m_vDivide;
	vec4 *m_vValue;
	float *m_vTime;
	int m_fps;
	int m_nPts;
};

struct SMVResourceAsyn
{
	void reset()
	{
		status = SES_IDLE;
	}

	void destory()
	{
		for (int i = 0; i < m_v2DEffectModel.size(); ++i)
		{
			m_v2DEffectModel[i].destory();
		}
		m_v2DEffectModel.clear();

		status = SES_IDLE;
	}

	//output
	vector<Effect2DRect> m_v2DEffectModel;

	StickerEffect_status status;
	string szPath;
	string szXMLFile;
};

class MVAction
{
public:
	MVAction();
	virtual ~MVAction();

	bool loadAction(const char *szAction);

	bool renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height);

	void reset();

	void destory();

	void destoryResource();
private:
	SLinearData m_rotateCurve;

	SMVResourceAsyn m_mvResource;
	HANDLE m_hLoadThread;

	DX11FBO *m_pFBO;
	RectDraw *m_rectDraw;

	ID3D11Texture2D *m_pTargetTexture;

	std::shared_ptr< CC3DTextureRHI> m_InputSRV;

	float m_fStartTime;
};

