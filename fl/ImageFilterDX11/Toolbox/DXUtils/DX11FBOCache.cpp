#include "DX11FBOCache.h"
#include <assert.h>

DX11FBOCache::DX11FBOCache()
{
}


DX11FBOCache::~DX11FBOCache()
{
	purgeAllUnassignedFrameBuffers();
}

DX11FBO *DX11FBOCache::fetchFrameBufferForSize(int width, int height)
{
	assert(width > 0 && height > 0);

	DX11FBO *fbo = NULL;

	char szBuffer[128];
	sprintf(szBuffer, "%d_%d", width, height);
	GPUImageFBOMap_::iterator it = m_mapFBOs.find(szBuffer);
	if (it != m_mapFBOs.end())
	{
		if (it->second.size() > 0)
		{
			fbo = it->second.front();
			it->second.pop_front();
			fbo->lock();
		}
		else
		{
			fbo = new DX11FBO();
			fbo->initWithTexture(width, height);
			fbo->lock();
		}
	}
	else
	{
		fbo = new DX11FBO();
		fbo->initWithTexture(width, height);
		fbo->lock();
	}

	return fbo;
}

void DX11FBOCache::returnFrameBufferToCache(DX11FBO *fbo)
{
	if (fbo == NULL)return;

	fbo->resetLockCount();

	int width = fbo->width();
	int height = fbo->height();
	char szBuffer[128];
	sprintf(szBuffer, "%d_%d", width, height);
	GPUImageFBOMap_::iterator it = m_mapFBOs.find(szBuffer);
	if (it != m_mapFBOs.end())
	{
		it->second.push_back(fbo);
	}
	else
	{
		m_mapFBOs.insert(make_pair(szBuffer, GPUImageFBOList_()));
		it = m_mapFBOs.find(szBuffer);
		it->second.push_back(fbo);
	}
}

void DX11FBOCache::purgeAllUnassignedFrameBuffers()
{
	GPUImageFBOMap_::iterator it = m_mapFBOs.begin();
	while (it != m_mapFBOs.end())
	{
		GPUImageFBOList_ &fbolist = it->second;
		GPUImageFBOList_::iterator itfbo = fbolist.begin();
		while (itfbo != fbolist.end())
		{
			(*itfbo)->destory();
			++itfbo;
		}
		fbolist.clear();

		++it;
	}

	m_mapFBOs.clear();
}
