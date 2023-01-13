#include "DynamicRHI.h"
#include "TextureRHI.h"
#include "MaterialTexRHI.h"

const float* GetMatData(const glm::mat4& mat)
{
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		return &glm::transpose(mat)[0][0];
	}
	else
	{
		return &mat[0][0];
	}
}


CC3DDynamicRHI::CC3DDynamicRHI()
{

}

CC3DDynamicRHI::~CC3DDynamicRHI()
{

}

std::shared_ptr<CC3DTextureRHI> CC3DDynamicRHI::CreateTexture(float r, float g, float b, float a)
{
	uint8 tmp[] = { (uint8)(r * 255),(uint8)(g * 255),(uint8)(b * 255),(uint8)(a * 255) };
	return CreateTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, 1, 1, tmp, 4, false,false);
}

std::shared_ptr<MaterialTexRHI> CC3DDynamicRHI::CreateAsynTextureZIP(void* hZip, const std::string& ImagePath, bool bGenMipmap /*= false*/)
{
	std::shared_ptr<MaterialTexRHI> TexRHI = std::make_shared<MaterialTexRHI>();
	TexRHI->CreateTextureFromZIP((HZIP)hZip, ImagePath, bGenMipmap);
	return TexRHI;
}

std::shared_ptr<MaterialTexRHI> CC3DDynamicRHI::CreateAsynTextureFromFile(const std::string& FileName, bool bGenMipmap /*=false*/)
{
	std::shared_ptr<MaterialTexRHI> TexRHI = std::make_shared<MaterialTexRHI>();
	TexRHI->CreateTextureFromFile(FileName, bGenMipmap);
	return TexRHI;
}

std::shared_ptr<MaterialTexRHI> CC3DDynamicRHI::CreateAsynTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer, int rowBytes /*= 0*/, bool bGenMipmap /*= false*/, bool bMultSample /*= false*/)
{
	std::shared_ptr<MaterialTexRHI> TexRHI = std::make_shared<MaterialTexRHI>();
	TexRHI->CreateTexture(format, BindFlags, width, height, pBuffer, rowBytes, bGenMipmap, bMultSample);
	return TexRHI;
}

std::shared_ptr<MaterialTexRHI> CC3DDynamicRHI::CreateAsynTexture(float r, float g, float b, float a)
{
	uint8 tmp[] = { (uint8)(r * 255),(uint8)(g * 255),(uint8)(b * 255),(uint8)(a * 255) };
	std::shared_ptr<MaterialTexRHI> TexRHI = std::make_shared<MaterialTexRHI>();
	TexRHI->CreateTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, 1, 1, tmp, 4, false, false);
	return TexRHI;
}

std::shared_ptr< CC3DTextureRHI> CC3DDynamicRHI::FetchTexture(const std::string& szFile, bool bGenMipmap)
{
	auto itFind = m_TexCache.find(szFile);
	if (itFind != m_TexCache.end())
	{
		return itFind->second;
	}

	std::shared_ptr< CC3DTextureRHI> TexRHI = CreateTextureFromFile(szFile, bGenMipmap);
	if (TexRHI)
	{
		m_TexCache.insert({ szFile,TexRHI });
	}
	return TexRHI;
}

void CC3DDynamicRHI::RecoredTexture(const std::string& szFile, std::shared_ptr< CC3DTextureRHI> TexRHI)
{
	if (TexRHI)
	{
		m_TexCache.insert({ szFile,TexRHI });
	}
}

std::shared_ptr< SwapChainRHI> CC3DDynamicRHI::CreateSwapChain()
{
	return std::shared_ptr< SwapChainRHI>();
}

