#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Matrices.h"
#include "Algorithm/MathUtils.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "EffectKernel/DXBasicSample.h"
#include "BaseDefine/Vectors.h"
#include "Algorithm/HeadManage.h"


BEGIN_SHADER_STRUCT(GaussianBlur,0)
    DECLARE_PARAM(Vector4,pParam)
	BEGIN_STRUCT_CONSTRUCT(GaussianBlur)
		IMPLEMENT_PARAM("pParam", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT;

class CBackGroundFilter : public CEffectPart, protected DXBaicSample
{
public:
	CBackGroundFilter();
	virtual ~CBackGroundFilter();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);

	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

private:

	std::shared_ptr<CC3DVertexBuffer> m_VerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> m_IndexBuffer;
	std::shared_ptr<class ShaderRHI> m_Shader;

	std::shared_ptr<CC3DTextureRHI> m_FaceMask;

	std::shared_ptr<HeadManage> m_HeadManage;
	DECLARE_SHADER_STRUCT_MEMBER(GaussianBlur);

	int m_Width = 0;
	int m_Height = 0;
	Vector2 m_Vertices[4];
	int m_nVerts;
	float m_Radius = 2.0;
	std::vector<float> g_TextureCoordinate = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
};

