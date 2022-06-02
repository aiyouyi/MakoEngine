#include "DynamicRHI.h"
#include "TextureRHI.h"

CC3DDynamicRHI::CC3DDynamicRHI()
{

}

CC3DDynamicRHI::~CC3DDynamicRHI()
{

}

std::shared_ptr<CC3DTextureRHI> CC3DDynamicRHI::CreateTexture(float r, float g, float b, float a)
{
	uint8 tmp[] = { (uint8)(r * 255),(uint8)(g * 255),(uint8)(b * 255),(uint8)(a * 255) };
	return CreateTexture(CC3DDynamicRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, 1, 1, tmp, 4, false,false);
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

