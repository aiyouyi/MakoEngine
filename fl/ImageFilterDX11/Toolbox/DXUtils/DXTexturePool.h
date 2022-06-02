#pragma once
#include <list>
#include <map>
#include <string>
#include "DX11Texture.h"
#include <mutex>


class DXTexturePool
{
public:
	DXTexturePool();
	~DXTexturePool();
	// rgba data or NULL
	DX11Texture* fetchDXTexture2D(char* data, int width, int height);
	void returnDXTexture2D(DX11Texture*);
	void returnAndRemoveDXTexture2D(DX11Texture*);
	void purgeAllUnassignedTextures();
	int count;
private:
	typedef std::list<DX11Texture*> TexList;
	typedef std::map<std::string, TexList> TexListMap;

	std::mutex m_Mutex;
	TexListMap m_TexListMap;
};

