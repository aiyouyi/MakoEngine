#ifndef CC3D_TextureRHI
#define CC3D_TextureRHI

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DTextureRHI
{
public:
	enum // Output Type
	{
		OT_NONE,
		OT_RENDER_TARGET,
		OT_DEPTH_STENCIL,
		OT_BUFFER_UNORDER_ACCESS,
		OT_TEXTURE_UNORDER_ACCESS,
		OT_MAX
	};
public:
	CC3DTextureRHI();
	virtual ~CC3DTextureRHI();

	virtual bool InitTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer = nullptr, int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false) { return false; };
	virtual bool InitTextureFromFile(const std::string szFile, bool bGenMipmap = false) { return false; }
	virtual bool InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false) { return false; }
	virtual bool InitTextureFromFileCPUAcess(const std::string szFile) { return false; }

	virtual uint32_t GetWidth()const = 0;
	virtual uint32_t GetHeight() const = 0;
};

#endif