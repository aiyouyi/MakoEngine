#pragma once
#include "Toolbox/Render/TextureRHI.h"
#include "common.h"
#include "Toolbox/irefptr.h"

class DX11Texture;

class DX11IMAGEFILTER_EXPORTS_CLASS DX11Texture2D : public CC3DTextureRHI
{
public:
	DX11Texture2D();
	virtual ~DX11Texture2D();

	virtual bool InitTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer = nullptr, int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false);
	virtual bool InitTexture1D(uint32_t format, int32_t width, void* pBuffer, int rowBytes);
	virtual bool InitTextureFromFile(const std::string szFile, bool bGenMipmap = false);
	virtual bool InitTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false);
	virtual bool InitTextureFromFileCPUAcess(const std::string szFile);
	virtual uint32_t GetWidth()const ;
	virtual uint32_t GetHeight() const;
	virtual bool updateTextureInfo(void* pBuffer, int w, int h);
	virtual void AttatchSRV(void* SRV);

	ID3D11Texture2D* GetNativeTex();
	ID3D11Texture1D* GetNativeTex1D();
	ID3D11ShaderResourceView* GetSRV();
	void Attatch(DX11Texture* Tex);
	

private:
	std::shared_ptr<DX11Texture> m_DX11Texture;
	DX11Texture* m_Attatched = nullptr;
	IRefPtr< ID3D11ShaderResourceView> m_SRV;
	IRefPtr< ID3D11Texture1D> m_Tex1D;
	int32_t m_Width1D = 0;
};