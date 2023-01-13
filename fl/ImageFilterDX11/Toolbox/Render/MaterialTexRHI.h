#pragma once
#include "BaseDefine/Define.h"
#include "Toolbox/zip/unzip.h"
#include "Toolbox/inc.h"

class CC3DTextureRHI;
class MaterialTexRHI
{
public:
	MaterialTexRHI();
	~MaterialTexRHI();

	void CreateTextureFromZIP(HZIP hZip, const std::string& ImagePath, bool bGenMipmap);
	void CreateTextureFromFile(const std::string& FileName, bool bGenMipmap);
	void CreateTexture(uint32_t format, uint32_t bindFlags, int32_t width, int32_t height, void* pBuffer, int rowBytes , bool bGenMipmap = false, bool bMultSample = false);
	void Bind(int nIndex);
	void BindVS(int nIndex);
	void UpdateTexture();

	int32_t GetWidth() const { return Width; }
	int32_t GetHeight() const { return Height; }

	std::shared_ptr<CC3DTextureRHI> GetTextureRHI() {
		return TextureRHI;
	}

	void UpdateTextureInfo(const uint8_t* pMaterial, int nWidth, int nHeight);

	void AttatchTextureRHI(std::shared_ptr<CC3DTextureRHI> TexRHI);

private:
	bool GenMipmap = false;
	std::shared_ptr<CC3DTextureRHI> TextureRHI;
	std::shared_ptr<uint8_t> TextureData;
	int32_t Width = 0;
	int32_t Height = 0;
	uint32_t Format = 0;
	uint32_t BindFlags = 0;
};