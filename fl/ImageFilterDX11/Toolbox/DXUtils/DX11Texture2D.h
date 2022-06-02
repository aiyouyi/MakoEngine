#pragma once
#include "Toolbox/Render/TextureRHI.h"
#include "common.h"

class DX11Texture;

class DX11Texture2D : public CC3DTextureRHI
{
public:
	DX11Texture2D();
	virtual ~DX11Texture2D();

	virtual bool InitTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer = nullptr, int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false);
	virtual bool InitTextureFromFile(const std::string szFile, bool bGenMipmap = false);
	virtual bool InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false);
	virtual bool InitTextureFromFileCPUAcess(const std::string szFile);
	virtual uint32_t GetWidth()const ;
	virtual uint32_t GetHeight() const;

	ID3D11Texture2D* GetNativeTex();
	ID3D11ShaderResourceView* GetSRV();
	void Attatch(DX11Texture* Tex);

private:
	std::shared_ptr<DX11Texture> m_DX11Texture;
	DX11Texture* m_Attatched = nullptr;
};