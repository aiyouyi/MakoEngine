#ifndef __GLCubeMap__
#define __GLCubeMap__

#include "Render/CubeMapRHI.h"

class GLCubeMap : public CC3DCubeMapRHI
{
public:
	GLCubeMap();
	virtual ~GLCubeMap();

	virtual bool Init(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget);

	virtual void SetRenderTarget(int32_t targetViewSlot, int32_t nMip = 0);
	virtual void ClearDepthBuffer();
	virtual void ClearRenderTarget(float fRed, float fGreen, float fBlue, float fAlpha);
	virtual void Destory();
	virtual void UnBindRenderTarget();

	uint32_t GetTextureId() const;
	GLuint GetFBO()const;

private:
	GLuint m_CubeTexId = 0;

	GLuint m_captureFBO = 0;
	GLuint m_captureRBO = 0;

	int32_t m_Width = 0;
	int32_t m_Height = 0;
};

#endif