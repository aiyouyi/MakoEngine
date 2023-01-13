#pragma once

#include "BaseDefine/Vectors.h"
#include "Toolbox/inc.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include <d3d11.h>

class DX11Texture;

class ShaderRHI;
class CC3DBlendState;

BEGIN_SHADER_STRUCT(ConstantBufferMask, 0)
	DECLARE_PARAM(glm::mat4, matWVP)
	DECLARE_PARAM(Vector4, clip)
	DECLARE_PARAM(Vector2, texSize)
	DECLARE_PARAM(Vector2, pad)
	BEGIN_STRUCT_CONSTRUCT(ConstantBufferMask)
		IMPLEMENT_PARAM("matWVP", UniformType::MAT4)
		IMPLEMENT_PARAM("clip", UniformType::FLOAT4)
		IMPLEMENT_PARAM("texSize", UniformType::FLOAT2)
		IMPLEMENT_PARAM("pad", UniformType::FLOAT2)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class DX11IMAGEFILTER_EXPORTS_CLASS RectDraw
{
public:
	RectDraw();
	virtual ~RectDraw();

	bool init(float width, float height, const std::string &szTexture = "");
	bool init(float x, float y, float width, float height, const std::string &szTexture = "");

	void reRect(float x, float y, float width, float height);

	void setBlend(bool bBlend);

	void setTexture(std::shared_ptr<CC3DTextureRHI> pTexture);

	void setShaderTextureView(std::shared_ptr<CC3DTextureRHI> pShaderTextureView);

	void renderOpaque();

	void render();

	void render(std::shared_ptr<CC3DTextureRHI> pShaderTextureView, float *arrClip, void *pMaskInfo, int w, int h);

	void renderAlpha(float *arrClip, void *pMaskInfo, int maskW, int maskH, int texW, int TexH);

	void render(Vector2 vTrans, Vector2 vScale, float fRotate, int w, int h);

	void destory();

protected:
	void updateBlendState();

private:

	std::shared_ptr<CC3DVertexBuffer> m_VertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> m_IndexBuffer;
	
	std::shared_ptr<ShaderRHI> m_pShader;
	std::shared_ptr<ShaderRHI> m_pShaderOpaque;
	std::shared_ptr<ShaderRHI> m_pShaderWithMask;
	std::shared_ptr<ShaderRHI> m_pShaderWithAlpha;


	std::shared_ptr<CC3DTextureRHI> m_pTexture;

	bool m_bInvalidBlendState = true;
	bool m_bBlend  = false;
	std::shared_ptr<CC3DBlendState> m_pBSState;

	std::shared_ptr<CC3DTextureRHI> m_pTextureMask;

	DECLARE_SHADER_STRUCT_MEMBER(ConstantBufferMat4);
	DECLARE_SHADER_STRUCT_MEMBER(ConstantBufferMask);

};

