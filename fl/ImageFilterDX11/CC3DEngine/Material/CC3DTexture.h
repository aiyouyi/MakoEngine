#pragma once
#ifndef _H_MT3D_TEXTURE_H_
#define _H_MT3D_TEXTURE_H_

#include "Common/CC3DUtils.h"
#include "ToolBox/GL/CCFrameBuffer.h"

class CC3DTexture 
{
public:
	CC3DTexture(void);
	virtual ~CC3DTexture(void);
	//初始化
	virtual void Initialize();
	//释放
	virtual void Release();

	bool LoadTexture(const char* filePath, bool useMipmap = false);
	bool LoadTexture(uint8* pSrcRGBA, uint32 width, uint32 height, bool useMipmap = false);
	bool LoadTextureRGBA(GLfloat* pSrcRGBA, uint32 width, uint32 height, GLenum InternalFormat, GLenum Format);

	bool LoadTexture(float r, float g, float b, float a);

	void Init(GLuint texture, int nWidth, int nHeight, bool bUseDepthBuffer = false);
	void Init(int nWidth, int nHeight, bool bUseDepthBuffer = false, GLint internalformat = GL_RGBA, GLenum format = GL_RGBA,
		GLenum type = GL_UNSIGNED_BYTE, bool bUseMultiTarget = false);

	void bindFBO();
	void unbindFBO();
	//当前所属ID
	uint32 GetGLTextureID();

	//当前所属ID
	uint32 GetSrcTextureID();

	//当前纹理宽
	uint32 GetTextureWidth();
	//当前纹理高
	uint32 GetTextureHeight();

	//纹理ID
	uint32 m_TextureID;
	//纹理宽
	uint32 m_nTextureWidth;
	//纹理高
	uint32 m_nTextureHeight;
	uint32 m_pTextureLoad = 0;

	CCFrameBuffer* m_FBO = NULL;
};

#endif // _H_MT3D_TEXTURE_H_
