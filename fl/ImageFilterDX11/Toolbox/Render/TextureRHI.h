#ifndef CC3D_TextureRHI
#define CC3D_TextureRHI

#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DTextureRHI
{
public:
	enum	//Surface Format Type
	{
		SFT_A8R8G8B8,
		SFT_A8B8G8R8,
		SFT_X8R8G8B8,
		SFT_D16,
		SFT_A32R32G32B32F,
		SFT_R32G32B32F,
		SFT_A16R16G16B16F,
		SFT_R16G16B16F,
		SFT_G16R16F,
		SFT_R16F,
		SFT_R32F,
		SFT_UNKNOWN,
		SFT_D24S8,
		SFT_R32G32F,
		SFT_R5G6B5,
		SFT_A8,
		SFT_R16G16,
		SFT_A16B16G16R16,
		SFT_R8,
		SFT_A8R8G8B8_S,
		SFT_G16R16_S,
		SFT_R16,
		SFT_A16B16G16R16_S,
		//Compress Type
		SFT_BC1,
		SFT_BC2,
		SFT_BC3,
		SFT_BC4,
		SFT_BC5,
		SFT_MAX
	};

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
	virtual bool InitTexture1D(uint32_t format, int32_t width, void* pBuffer, int rowBytes) { return false; }
	virtual bool InitTextureFromFile(const std::string szFile, bool bGenMipmap = false) { return false; }
	virtual bool InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false) { return false; }
	virtual bool InitTextureFromFileCPUAcess(const std::string szFile) { return false; }

	virtual uint32_t GetWidth()const = 0;
	virtual uint32_t GetHeight() const = 0;

	virtual void AttatchTextureId(uint32_t id) {};
	virtual void AttatchTextureId(std::shared_ptr< CC3DTextureRHI> TexRHI) {};
	virtual bool updateTextureInfo(void* pBuffer, int w, int h) { return false; }
	virtual void AttatchSRV(void* SRV) {}
};

#endif