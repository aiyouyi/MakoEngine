#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Matrices.h"
#include "Algorithm/MathUtils.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "EffectKernel/DXBasicSample.h"
#include "BaseDefine/Vectors.h"
#include "Algorithm/HeadManage.h"


BEGIN_SHADER_STRUCT(FGTrackConstBuffer,0)
    DECLARE_PARAM(Vector4,pParam)
	DECLARE_PARAM(Vector4,pClipParam, Vector4(0, 0, 1.0, 0))
	BEGIN_STRUCT_CONSTRUCT(FGTrackConstBuffer)
		IMPLEMENT_PARAM("pParam", UniformType::FLOAT4)
	    IMPLEMENT_PARAM("pClipParam", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT;

class CBodyFGTrackEffect : public CEffectPart, protected DXBaicSample
{
public:
	CBodyFGTrackEffect();
	virtual ~CBodyFGTrackEffect();
	virtual void Release();
	virtual void* Clone();

	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool ReadConfig(XMLNode& childNode, const std::string& path);

	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	Vector2 m_SrcRect[4];
private:
	void WarpTransform(Vector2 *dst);
	void ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL, const std::string& path = "");
	void Resize(int nWidth, int nHeight);
	std::array<std::array<std::shared_ptr<CC3DVertexBuffer>, 10>,10> m_VerticeBuffer;
	std::shared_ptr<CC3DVertexBuffer> m_SplitVerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> m_IndexBuffer;
	std::shared_ptr<class ShaderRHI> m_Shader;
	std::shared_ptr<class ShaderRHI> m_ShaderSplit;

	DECLARE_SHADER_STRUCT_MEMBER(FGTrackConstBuffer); 

	int m_Width = 0;
	int m_Height = 0;
	Vector2 Delta;
	float LastAlpha = 1.0;
	Vector2 lastDstPoint[3][3];

	int m_ModelWidth = 948;
	int m_ModelHeight = 533;

	bool m_DisableRoate = false;

	Vector2 m_Vertices[4];
	int m_nVerts;
	Vector2 m_SrcAbsRect[4];
	Vector2 m_DstPoint[3];
	Vector2 m_SrcPoint[3] = { Vector2(402, 196), Vector2(478, 39), Vector2(548, 206) };

	std::vector<float> g_TextureCoordinate = { 0.0f,1.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f };
};