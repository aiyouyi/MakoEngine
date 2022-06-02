#include "DXTexturePool.h"
#include <assert.h>


DXTexturePool::DXTexturePool()
{
	count = 0;
}

DXTexturePool::~DXTexturePool() {
	purgeAllUnassignedTextures();
}

DX11Texture* DXTexturePool::fetchDXTexture2D(char* data, int width, int height) {
	std::lock_guard<std::mutex> lock(m_Mutex);
	assert(width > 0 && height > 0);

	DX11Texture* tex = NULL;

	char szBuffer[128];
	sprintf(szBuffer, "%d_%d", width, height);
	TexListMap::iterator it = m_TexListMap.find(szBuffer);
	if (it != m_TexListMap.end())
	{
		if (it->second.size() > 0)
		{
			tex = it->second.front();
			it->second.pop_front();
		}
		else
		{
			count++;
			tex = new DX11Texture();
			HRESULT hr = tex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, width, height, data, width * 4, false, false);
			if (FAILED(hr))
			{
				return NULL;
			}
		}
	}
	else
	{
		count++;
		tex = new DX11Texture();
		HRESULT hr = tex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, width, height, data, width * 4, false, false);
		if (FAILED(hr))
		{
			return NULL;
		}
	}
	assert(tex != NULL && tex->getTex() != NULL);
	return tex;
}

void DXTexturePool::returnDXTexture2D(DX11Texture* tex)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	assert(tex != NULL && tex->getTex() != NULL);

	int width = tex->width();
	int height = tex->height();
	char szBuffer[128];
	sprintf(szBuffer, "%d_%d", width, height);
	TexListMap::iterator it = m_TexListMap.find(szBuffer);
	if (it != m_TexListMap.end())
	{
		it->second.push_back(tex);
	}
	else
	{
		m_TexListMap.insert(make_pair(szBuffer, TexList()));
		it = m_TexListMap.find(szBuffer);
		it->second.push_back(tex);
	}
}

void DXTexturePool::returnAndRemoveDXTexture2D(DX11Texture* tex)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (NULL == tex) return;
	assert(tex->getTex() != NULL);
	tex->destory();
}

void DXTexturePool::purgeAllUnassignedTextures()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	TexListMap::iterator it = m_TexListMap.begin();
	while (it != m_TexListMap.end())
	{
		TexList& texlist = it->second;
		TexList::iterator ittex = texlist.begin();
		while (ittex != texlist.end())
		{
			(*ittex)->destory();
			delete (*ittex);
			++ittex;
		}
		texlist.clear();

		++it;
	}
	m_TexListMap.clear();	
}

