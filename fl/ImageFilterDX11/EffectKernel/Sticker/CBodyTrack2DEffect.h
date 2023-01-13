#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Drawable.hpp"
//#include "EffectModel.hpp"
#include "Toolbox/DXUtils/DXUtils.h"
#include <vector>
#include "EffectKernel/DXBasicSample.h"

class CBodyTrack2DEffect :public CEffectPart, protected DXBaicSample
{
public:
	struct VideodrawableXMLInfo
	{
		std::string Material;
		std::string Track;
		std::string blendType;
		std::string EnableMp4Alpha;
	};
public:
	CBodyTrack2DEffect();
	virtual ~CBodyTrack2DEffect();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool ReadConfig(XMLNode& childNode, const std::string &path);
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);

	int GetBodyIndex(BaseRenderParam& RenderParam,int SplitNum, int SplitIndex);
	Vector2 SrcPoint[3];
	Vector2 LastDes[3][3];
	
	bool m_DisableRoate = false;
	float m_RoateValue = 0.0;
	float LastAlpha=1.0;
	ID3D11Buffer*       m_pConstantBufferMask;  
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;

	__int64 m_nStartTime;
	__int64 m_nLoopStartTime;
	__int64 m_nLastTime;
	long m_nEffectDuring;
	long m_nAniLoopTime;

	Vector2 m_SrcVertexPoint[3] = { Vector2(0.0,0.0)* 2.0 - 1.0,Vector2(0.0,1.0)* 2.0 - 1.0,Vector2(1.0,1.0)* 2.0 - 1.0 };
	Vector2 m_DstVertexPoint[3][3];
	bool m_IsTrack = true;
	VideodrawableXMLInfo m_VideoXMLInfo;
};

