#pragma once
#ifndef _H_CC3D_MAKEUP_H_
#define _H_CC3D_MAKEUP_H_
#include "Material/CC3DTexture.h"
#include "ToolBox/GL/CCFrameBuffer.h"
#include "ToolBox/GL/FilterBase.h"
#include "ToolBox/Render/TextureRHI.h"



class CC3DMakeUpGL 
{
public:
	enum CC3DMakeUpType
	{
		CC3D_EYEBROW,
		CC3D_EYE,
		CC3D_MOUTH,
		CC3D_BEARD,
	};

	enum CC3DBlendType
	{
		CC3D_NORAML,
		CC3D_MULTIPLY,
		CC3D_SOFTLIGHT,
	};

	class CC3DMakeUpResource
	{
	public:
		CC3DMakeUpType type;
		CC3DBlendType   blendType;
		CC3DTexture* m_pTexture;
		Vector2 m_Vertices[4];
	};
public:
	CC3DMakeUpGL();
	~CC3DMakeUpGL();

	void initMainTexture(std::shared_ptr<CC3DTextureRHI> pTexture, std::string path);

	void DeleteTexture(CC3DMakeUpType type);

	void PushTexture(BYTE *pRGBA, int nWidth, int nHeight, Vector2 *pRectVertices, CC3DMakeUpType type);

	void SetBlendType(CC3DMakeUpType type, CC3DBlendType blendType);

	void RenderMakeUp();

	void ChangeColor(float r, float g, float b, float a, CC3DTexture *pTexture, CC3DMakeUpType type = CC3D_EYEBROW);

	virtual void update();

	bool m_NeedFlush = false;
	CCFrameBuffer *m_FBO;
private:

	CCProgram *m_pShader;
	CCProgram *m_pShaderColor = NULL;
	std::shared_ptr<CC3DTextureRHI> m_pMainTexture;

	std::vector<CC3DMakeUpResource> m_AllResource;

	FilterBase m_filter;

};

#endif // _H_CC3D_MAKEUP_H_
