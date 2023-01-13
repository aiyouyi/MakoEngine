#ifndef CCFRAMEBUFFER_H
#define CCFRAMEBUFFER_H

#include"openglutil.h"

class CCFrameBuffer
{
public:
    CCFrameBuffer();
    CCFrameBuffer(GLuint fbo, GLuint id, int nWidth, int nHeight);
    virtual ~CCFrameBuffer();
	virtual void release();

	virtual void bind();
	virtual void unbind();
	virtual void init(int nWidth, int nHeight, bool bUseDepthBuffer = false, GLint internalformat = GL_RGBA, GLenum format = GL_RGBA, 
		              GLenum type = GL_UNSIGNED_BYTE,bool bUseMultiTarget = false);
	virtual void init(GLuint texture, int nWidth, int nHeight,bool bUseDepthBuffer = false);
	
	virtual void ReadPixels(BYTE * pData);

	virtual void AttatchColorBuffers(int nCount);
	virtual void ClearColor();
	virtual void ClearDepth();
private:
	void generateTexture(GLint internalformat, GLenum format, GLenum type);
public:
    GLuint framebufferID;
    GLuint textureID;
	GLuint secondID;
    GLuint renderBufferID;
    int width;
    int height;
    bool needReleaseTexture;
    bool needReleaseFramebuffer;
    bool onlyTexture;
	bool isMultiTarget;
};

#endif // CCFRAMEBUFFER_H
