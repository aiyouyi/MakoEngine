#pragma once
#include "DX11Texture.h"
#include "DX11FBOCache.h"
#include "DX11Shader.h"
#include <d3d11.h>
#include <map>
using namespace std;

#define DeviceContextPtr (DX11Context::shareInst()->getDeviceContext())
#define DevicePtr (DX11Context::shareInst()->getDevice())
#define ContextInst (DX11Context::shareInst())
#define FBOCacheInst (DX11Context::shareInst()->getFBOCache())

class DX11IMAGEFILTER_EXPORTS_CLASS DX11Context
{
private:
	DX11Context();
	virtual ~DX11Context();
	static DX11Context *s_pInst;
public:
	static DX11Context *shareInst();
	static void destoryInst();

	bool initContext(ID3D11Device *pd3dDevice,ID3D11DeviceContext *pContext);
	void uninit();
	ID3D11Device *getDevice();
	ID3D11DeviceContext *getDeviceContext();

	DX11FBOCache *getFBOCache();

	DX11Texture *fetchTexture(const string &szTexture, bool bGenMipmap=false);

	DX11Shader *fetchShader(const string &szShader, bool defaultCreate=false);

	//外部自己手动创建的纹理也可以进行记录，供其他引用
	void recordTexture(const string &szTexture, DX11Texture *pTexture);

	//外部自己手动创建的纹理也可以进行记录，供其他引用
	void recordShader(const string &szShader, DX11Shader *pShader);

	void clearUnrefResource();

	ID3D11DepthStencilState *fetchDepthStencilState(bool enableDepthTest, bool enableDepthWrite);

	ID3D11BlendState *fetchBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer);

	ID3D11RasterizerState *fetchRasterizerState(D3D11_CULL_MODE cullMode);

	void setCullMode(D3D11_CULL_MODE cullMode);

private:
	ID3D11Device *m_pDevice;
	ID3D11DeviceContext *m_pContext;

	typedef map<string, DX11Texture *> GLTextureMap_;

	typedef map<string, DX11Shader *> GLShaderMap_;

	typedef map<unsigned int, ID3D11DepthStencilState *> DepthStencilStateMap_;

	typedef map<unsigned int, ID3D11BlendState *> BlendStateMap_;

	typedef map<unsigned int, ID3D11RasterizerState *> RasterizerStateMap_;

	CRITICAL_SECTION m_cs;

	GLTextureMap_ m_mapTextures;

	GLShaderMap_ m_mapShaders;

	DepthStencilStateMap_ m_mapDepthStencialState;

	BlendStateMap_ m_mapBlendState;

	RasterizerStateMap_ m_mapRasterizerState;

	DX11FBOCache *m_pFBOCache;
};

