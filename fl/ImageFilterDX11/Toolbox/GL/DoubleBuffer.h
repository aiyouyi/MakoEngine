#pragma once
#include "Toolbox/GL/openglutil.h"
#include "Toolbox/GL/FilterBase.h"
#include "Toolbox/GL/CCFrameBuffer.h"
#include "Toolbox/Render/DoubleBufferRHI.h"

class GLTexture2D;
class CC3DTextureRHI;

class GLDoubleBuffer : public DoubleBufferRHI
{
public:
	GLDoubleBuffer();
	~GLDoubleBuffer();
	void Release();

    void InitDoubleBuffer(GLuint TexIDA,int nWidth, int nHeight,bool UseDepth = false);
    void UpdtaeTexA(GLuint TexIDA);

   	void InitDoubleBuffer(int nWidth, int nHeight, bool UseDepth = false, GLint internalformat = GL_RGBA, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

protected:
	virtual void InitDoubleBuffer(uint32_t format, int32_t width, int32_t height, bool UseDepth, bool UseMultiTarget, bool bMSAA) override;
public:

	void BindFBOA();
    void unBindFBOA();

	void BindFBOB();
    void unBindFBOB();

	void SwapFBO();

	void SyncAToB();
	void SyncBToA();

	void SyncAToBRegion(float *pVerts, int nVerts, int nStep = 2, int type = 0);

	int GetWidth() const override;
	int GetHeight()const override;
public:
	std::shared_ptr<CC3DTextureRHI> GetFBOTextureA() override;
	std::shared_ptr<CC3DTextureRHI> GetFBOTextureB() override;

	GLuint GetFBOTextureAID();
	GLuint GetFBOTextureBID();

	GLuint GetFBOA();
	GLuint GetFBOB();
	CCFrameBuffer* GetFBOABuffer();
	CCFrameBuffer* GetFBOBBuffer();

    void FilterToTex(GLuint InptuTex);
    void FilterToGray(GLuint InptuTex,int nWidth,int nHeight);
	
private:
	
	int m_nWidth;
	int m_nHeight;

	std::shared_ptr< GLTexture2D> m_FBOB;
	std::shared_ptr< GLTexture2D> m_FBOA;

    FilterBase *m_filter;
};

