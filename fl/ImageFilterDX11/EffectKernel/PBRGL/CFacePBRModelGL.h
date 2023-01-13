#ifndef CFacePBR_H
#define CFacePBR_H
#include "EffectKernel/CEffectPart.h"
#include "Scene/CC3DSceneManage.h"
#include "Render/CC3DPbrRender.h"
#include <vector>
#include <memory>
#include "Toolbox/Render/CC3DShaderDef.h"

class CC3DSceneManage;
class CC3DPbrRenderGL;
class CRenderPBRModelGL;
class Drawable;
class CCProgram;

BEGIN_SHADER_STRUCT(HeadCull, 0)
	DECLARE_PARAM(glm::mat4, matWVP)
	DECLARE_PARAM(Vector4, faceOriRect)
	BEGIN_STRUCT_CONSTRUCT(HeadCull)
		IMPLEMENT_PARAM("u_matWVP", UniformType::MAT4)
		IMPLEMENT_PARAM("faceoriRect", UniformType::FLOAT4)
	END_SHADER_STRUCT
END_SHADER_STRUCT
;
class CFacePBRModelGL :public CEffectPart
{
public:
	CFacePBRModelGL();
	virtual ~CFacePBRModelGL();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;
	
	void UpdateEmissMap();
	std::shared_ptr<CRenderPBRModelGL> m_RenderUtil;
private:
	//渲染模型信息
	std::shared_ptr<CCProgram> m_pShaderForHeaderCull;
    std::shared_ptr<CC3DVertexBuffer> VerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer;

	//vector<MatrialVideo>m_MatrialVideo;
	int m_nCamIndex = 0;
	int hdrWidth = 0;
	int hdrHeight = 0;
	float m_rotateX = 0.0f;
	float m_MatScaleHead = 1.0f;
	float m_MatScale = 100.f;

	DECLARE_SHADER_STRUCT_MEMBER(HeadCull);
};

#endif