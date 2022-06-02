#ifndef GPUImageFramebufferCache_h
#define GPUImageFramebufferCache_h
#include "DX11FBO.h"
#include <string>
#include <map>
#include <list>
using namespace std;

class DX11IMAGEFILTER_EXPORTS_CLASS DX11FBOCache
{
public:
	DX11FBO *fetchFrameBufferForSize(int width, int height);

	void returnFrameBufferToCache(DX11FBO *fbo);

	void purgeAllUnassignedFrameBuffers();

	friend class DX11Context;
protected:
	DX11FBOCache();
	~DX11FBOCache();

private:
	typedef list<DX11FBO *> GPUImageFBOList_;
	typedef map<string, GPUImageFBOList_> GPUImageFBOMap_;

	GPUImageFBOMap_ m_mapFBOs;
};
#endif