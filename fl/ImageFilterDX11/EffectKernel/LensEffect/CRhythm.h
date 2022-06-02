#pragma once

#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Toolbox/Render/DynamicRHI.h"

class MAGICSTUDIO_EXPORTS_CLASS CRhythm: public CEffectPart
{
public:
	CRhythm();
    virtual ~CRhythm();
    virtual void Release();
    virtual void* Clone();
    virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool ReadConfig(XMLNode & childNode, const std::string & path);
    virtual bool Prepare();
    virtual void Render(BaseRenderParam &RenderParam);

	void SetEffectCount(int count);
	float m_Alltime = 2000;
	float m_DuringTime = 500;
	int m_Loop = 0;
	long m_StartTime = 0;

	//TODO:还有数组类型的uniform没有解决，好像可以直接使用size去包含整个数组大小
	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(ConstantBufferVec4).SetParameter(name, value, size);
	}

	float m_RealScale = 1;
	Vector2 m_RealCenterPoint = Vector2(0.5, 0.5);
	bool m_Stable = false;

private:
	bool ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path);
	bool WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src);

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;
	DELCARE_SHADER_STRUCT_MEMBER(ConstantBufferVec4);

	float MaxScale = 1.8;
	float MidScale = 1.4;
	float ScaleSmooth = 0.04;
	float PosSmooth = 0.066;
};
