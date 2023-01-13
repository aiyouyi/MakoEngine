#include "MaterialTexRHI.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/TextureRHI.h"

MaterialTexRHI::MaterialTexRHI()
{

}

MaterialTexRHI::~MaterialTexRHI()
{

}

void MaterialTexRHI::CreateTextureFromZIP(HZIP hZip, const std::string& ImagePath, bool bGenMipmap)
{
	Format = CC3DTextureRHI::SFT_A8R8G8B8;
	BindFlags = CC3DTextureRHI::OT_NONE;

	uint8_t* Data = ::CreateImgFromZIP(hZip, ImagePath.c_str(), Width, Height);
	TextureData = std::shared_ptr<uint8_t>(Data, [](uint8_t *p) {delete[]p; });
	GenMipmap = bGenMipmap;
}

void MaterialTexRHI::CreateTextureFromFile(const std::string& FileName, bool bGenMipmap)
{
	Format = CC3DTextureRHI::SFT_A8R8G8B8;
	BindFlags = CC3DTextureRHI::OT_NONE;

	uint8_t* Data = ::LoadImageFromFile(FileName, Width, Height);
	TextureData = std::shared_ptr<uint8_t>(Data, [](uint8_t* p) {delete[]p; });
	GenMipmap = bGenMipmap;
}

void MaterialTexRHI::CreateTexture(uint32_t format, uint32_t bindFlags, int32_t width, int32_t height, void* pBuffer, int rowBytes , bool bGenMipmap /*= false*/, bool bMultSample /*= false*/)
{
	Format = format;
	BindFlags = bindFlags;
	Width = width;
	Height = height;

	uint8_t* Data = new uint8_t[rowBytes * Height];
	memcpy(Data, pBuffer, rowBytes * Height);
	TextureData = std::shared_ptr<uint8_t>(Data, [](uint8_t* p) {delete[]p; });
	GenMipmap = bGenMipmap;
}

void MaterialTexRHI::Bind(int nIndex)
{
	UpdateTexture();
	GetDynamicRHI()->SetPSShaderResource(nIndex, TextureRHI);
}

void MaterialTexRHI::BindVS(int nIndex)
{
	UpdateTexture();
	GetDynamicRHI()->SetVSShaderResource(nIndex, TextureRHI);
}

void MaterialTexRHI::UpdateTexture()
{
	if (!TextureRHI)
	{
		TextureRHI = GetDynamicRHI()->CreateTexture(Format, BindFlags, Width, Height, TextureData.get(), Width*4, GenMipmap);
		TextureData.reset();
	}
}

void MaterialTexRHI::UpdateTextureInfo(const uint8_t* pMaterial, int nWidth, int nHeight)
{
	Width = nWidth;
	Height = nHeight;
	TextureData.reset();
	if (!TextureRHI)
	{
		TextureRHI = GetDynamicRHI()->CreateTexture(Format, BindFlags, Width, Height, const_cast<uint8_t*>(pMaterial), Width * 4, GenMipmap);
	}
	else
	{
		TextureRHI->updateTextureInfo(const_cast<uint8_t*>(pMaterial), nWidth, Height);
	}
}

void MaterialTexRHI::AttatchTextureRHI(std::shared_ptr<CC3DTextureRHI> TexRHI)
{
	TextureData.reset();
	Width = TexRHI->GetWidth();
	Height = TexRHI->GetHeight();
	Format = 0;
	BindFlags = 0;
	TextureRHI = TexRHI;
}
