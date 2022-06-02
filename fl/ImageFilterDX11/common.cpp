#include "common.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/DXUtils/DX11DynamicRHI.h"
#include "CocosARRender\renderer\CShaderCache.h"
#include "base/CCAutoreleasePool.h"

#if _MSC_VER  
#include <Windows.h>
#else
#include <sys/time.h>
#endif

DX11IMAGEFILTER_EXPORTS_API unsigned long ccCurrentTime()
{
#if (defined _WIN64) || (defined _WIN32)  
	return GetTickCount();
#else
	static bool bFirst = true;
	static struct timeval tvBegin;
	struct timeval tvEnd;
	if (bFirst)
	{
		bFirst = false;
		gettimeofday(&tvBegin, NULL);
	}

	gettimeofday(&tvEnd, NULL);

	return (tvEnd.tv_usec - tvBegin.tv_usec) / 1000 + (tvEnd.tv_sec - tvBegin.tv_sec) * 1000;
#endif
}

DX11IMAGEFILTER_EXPORTS_API void ccBeginFilter()
{
}

DX11IMAGEFILTER_EXPORTS_API void ccEndFilter()
{
	ContextInst->clearUnrefResource();
}

DX11IMAGEFILTER_EXPORTS_API void ccInitFilter(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pContext)
{
	ContextInst->initContext(pd3dDevice, pContext);
	CC3DPiplelineState::Initialize();
	GetDynamicRHI();
}

DX11IMAGEFILTER_EXPORTS_API void ccUnInitFilter()
{
	CC3DPiplelineState::DestroyAll();
	DX11DynamicRHI::Release();
	ContextInst->uninit();
	cocos2d::CCShaderCache::destoryInst();

	ContextInst->destoryInst();

	cocos2d::PoolManager::getInstance()->getCurrentPool()->clear();
	cocos2d::PoolManager::destroyInstance();
}