#ifndef __GLTexture2d__
#define __GLTexture2d__

#include "Render/TextureRHI.h"
#include "Toolbox/GL/openglutil.h"

class CC3DTexture;

class GLTexture2D : public CC3DTextureRHI
{
public:
	GLTexture2D();
	virtual ~GLTexture2D();

	virtual bool InitTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer = nullptr, int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false);
	virtual bool InitTextureFromFile(const std::string szFile, bool bGenMipmap = false);
	virtual bool InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false);
	virtual bool InitTextureFromFileCPUAcess(const std::string szFile);

	virtual uint32_t GetWidth()const;
	virtual uint32_t GetHeight() const;

	void InitFrameBuffer(uint32_t texture, int nWidth, int nHeight, bool bUseDepthBuffer = false);
	void InitFrameBuffer(int nWidth, int nHeight, bool bUseDepthBuffer = false, GLint internalformat = GL_RGBA, GLenum format = GL_RGBA,
		GLenum type = GL_UNSIGNED_BYTE, bool bUseMultiTarget = false);

	std::shared_ptr<CC3DTexture> Get3DTexture() { return m_Texture; }
	uint32_t GetTextureID() const;
	uint32_t GetFrameBufferID() const;

	virtual void AttatchTextureId(uint32_t id) override;
	virtual void AttatchTextureId(std::shared_ptr< CC3DTextureRHI> TexRHI) override;
	virtual bool updateTextureInfo(void* pBuffer, int w, int h) override;

private:
	std::shared_ptr<CC3DTexture> m_Texture;
	bool m_NeedToRelease = true;
};

#endif