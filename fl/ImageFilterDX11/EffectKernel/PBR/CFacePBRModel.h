#pragma once
#include "EffectKernel/CEffectPart.h"
#include "CC3DEngine/Render/CRenderPBRModel.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "EffectKernel/DXBasicSample.h"

class CFacePBRModel :public CEffectPart, protected DXBaicSample
{
public:
	CFacePBRModel();
	~CFacePBRModel();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;
	virtual bool WriteConfig(std::string& tempPath, XMLNode& root, HZIP dst = 0, HZIP src = 0) override;
	//TODO:还有数组类型的uniform没有解决，好像可以直接使用size去包含整个数组大小

	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).SetParameter(name, value, size);
	}

	std::shared_ptr<CRenderPBRModel>m_RenderUtils;
	
private:
	std::shared_ptr<CC3DVertexBuffer> VerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer;
	float m_MatScale = 100.f;
	float m_MatScaleHead = 1.f;
	float m_rotateX = 0.0f;
	int m_HasRotate = 1;
	float m_GradientTime[3] = { 0,0,0 };
	DECLARE_SHADER_STRUCT_MEMBER(ConstantBufferMat4);
};

