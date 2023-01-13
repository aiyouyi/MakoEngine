#ifndef _CPURPLEFIRE_EFFECT_H_
#define _CPURPLEFIRE_EFFECT_H_

#include "EffectKernel/CEffectPart.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include <vector>
#include <memory>
#include "EffectKernel/DXBasicSample.h"
#include "Toolbox/Render/CC3DShaderDef.h"

BEGIN_SHADER_STRUCT(FirePassConstBuffer, 0)
	DECLARE_PARAM_VALUE(Vector4, param, Vector4(0,0,0,0))
	DECLARE_PARAM_VALUE(Vector4, thres, Vector4(0.85, 0, 0, 0))
	BEGIN_STRUCT_CONSTRUCT(FirePassConstBuffer)
		IMPLEMENT_PARAM("param", UniformType::FLOAT4)
	    IMPLEMENT_PARAM("thres", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(BlendPassConstBuffer, 0)
	DECLARE_PARAM_VALUE(Vector4, clip, Vector4(1, 0, 0, 0))
	DECLARE_PARAM_VALUE(Vector4, thres1, Vector4(0.85, 0, 0, 0))
	BEGIN_STRUCT_CONSTRUCT(BlendPassConstBuffer)
		IMPLEMENT_PARAM("clip", UniformType::FLOAT4)
	    IMPLEMENT_PARAM("thres1", UniformType::FLOAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class ShaderRHI;
class CC3DRenderTargetRHI;
class CC3DTextureRHI;

class CPurpleFire2DEffect :public CEffectPart, protected DXBaicSample
{
public:
	CPurpleFire2DEffect();
	virtual ~CPurpleFire2DEffect();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool ReadConfig(XMLNode& childNode, const std::string& path);
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

private:
	bool ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath, const std::string& path);
	bool WriteConfig(std::string& tempPath, XMLNode& root, HZIP dst, HZIP src);
	void InitRenderTarget();
private:
	std::shared_ptr<class CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<class CC3DIndexBuffer> mIndexBuffer;

	std::shared_ptr<ShaderRHI> velocityPassShader;
	std::shared_ptr<ShaderRHI> firePassShader;
	std::shared_ptr<ShaderRHI> blendPassShader;
	std::shared_ptr<ShaderRHI> normalPassShader;

	std::shared_ptr<CC3DRenderTargetRHI> lastInputImageRT;
	std::shared_ptr<CC3DRenderTargetRHI> lastFireImageRT;
	std::shared_ptr<CC3DRenderTargetRHI> lastFireImageRTHalfRes;

	std::shared_ptr<CC3DRenderTargetRHI> velocityRT;
	std::shared_ptr<CC3DRenderTargetRHI> velocityRTHalfRes;

	std::shared_ptr<class MaterialTexRHI> noiseTex;

	int mWidth = 0;
	int mHeight = 0;
	Vector4 fireParam = Vector4(0,0,0.0035, 0.0045);
	Vector4 clipParam = Vector4(1, 0, 0.5, 0.55);
	float m_ValueThres = 0.85;
	DECLARE_SHADER_STRUCT_MEMBER(FirePassConstBuffer);
	DECLARE_SHADER_STRUCT_MEMBER(BlendPassConstBuffer);
	int FrameIndex = 0;

};

#endif