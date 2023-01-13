#pragma once
#ifndef CFACESTICKER_H
#define CFACESTICKER_H

#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Render/CC3DShaderDef.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "GL/GLDynamicRHI.h"

BEGIN_SHADER_STRUCT(FaceSticker, 0)
	DECLARE_PARAM(float, m_alpha)
	DECLARE_PARAM(int32_t, m_BlendType)
	BEGIN_STRUCT_CONSTRUCT(FaceSticker)
		IMPLEMENT_PARAM("alpha", UniformType::FLOAT)
		IMPLEMENT_PARAM("blendtype", UniformType::INT)
	    INIT_TEXTURE_INDEX("inputImageTexture", 0)
	    INIT_TEXTURE_INDEX("inputImageTexture2", 1)
	END_SHADER_STRUCT
END_SHADER_STRUCT
;

class CCProgram;
class Drawable;

class CFaceEffect2DStickerGL : public CEffectPart
{
public:
	CFaceEffect2DStickerGL();
	virtual ~CFaceEffect2DStickerGL();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char *pFilePath = NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

private:
	std::shared_ptr<CC3DVertexBuffer> VerticeBuffer;
	std::shared_ptr<CCProgram> m_pStickerShader;
	std::shared_ptr<CC3DTextureRHI> RenderSrcRHI;

	std::shared_ptr<Drawable> m_Drawable;
	Vector2 m_SrcRect[4];

	int m_nVerts = 4;

	Vector2 m_DstPoint[3];
	Vector2 m_SrcPoint[3] = { Vector2(0.386523f * 930, 0.429125f * 1240), Vector2(0.599048f * 930, 0.427873f * 1240), Vector2(0.498499f * 930, 0.544121f * 1240) };

	std::vector<float> g_TextureCoordinate = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
	std::vector<unsigned short> g_Index ={ 
		0, 1, 2,
		1, 2, 3 };

	bool m_KeepShape = false;
	Vector2 m_SrcPointRect[3] = { Vector2(241,504.6), Vector2(241, 877.8), Vector2(678.4,877.8) };

	DECLARE_SHADER_STRUCT_MEMBER(FaceSticker);
};

#endif
