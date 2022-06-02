#include "DX11Context.h"

DX11Context *DX11Context::s_pInst = NULL;
DX11Context::DX11Context()
{
	m_pDevice = NULL;
	m_pContext = NULL;
	InitializeCriticalSectionAndSpinCount(&m_cs, 4000);

	m_pFBOCache = NULL;
}

DX11Context::~DX11Context()
{
	uninit();
	DeleteCriticalSection(&m_cs);
}

DX11Context *DX11Context::shareInst()
{
	if (s_pInst == NULL)
	{
		s_pInst = new DX11Context();
	}
	return s_pInst;
}

void DX11Context::destoryInst() 
{
	if (s_pInst != NULL)
	{
		delete s_pInst;
		s_pInst = NULL;
	}
}

bool DX11Context::initContext(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pContext)
{
	uninit();

	m_pFBOCache = new DX11FBOCache();

	m_pDevice = pd3dDevice;
	if (m_pDevice != NULL)
	{
		m_pDevice->AddRef();
	}

	m_pContext = pContext;
	if (m_pContext != NULL)
	{
		m_pContext->AddRef();
	}

	return true;
}

void DX11Context::uninit()
{
	clearUnrefResource();

	DepthStencilStateMap_::iterator it = m_mapDepthStencialState.begin();
	while (it != m_mapDepthStencialState.end())
	{
		if (it->second != NULL)
		{
			it->second->Release();
		}
		++it;
	}
	m_mapDepthStencialState.clear();
	

	BlendStateMap_::iterator itBlend = m_mapBlendState.begin();
	while(itBlend != m_mapBlendState.end())
	{
		if (itBlend->second != NULL)
		{
			itBlend->second->Release();
		}
		++itBlend;
	}
	m_mapBlendState.clear();


	RasterizerStateMap_::iterator itRasterizer = m_mapRasterizerState.begin();
	while (itRasterizer != m_mapRasterizerState.end())
	{
		if (itRasterizer->second != NULL)
		{
			itRasterizer->second->Release();
		}
		++itRasterizer;
	}
	m_mapRasterizerState.clear();


	if (m_pDevice != NULL)
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}

	if (m_pContext != NULL)
	{
		m_pContext->Release();
		m_pContext = NULL;
	}

	if (m_pFBOCache != NULL)
	{
		delete m_pFBOCache;
		m_pFBOCache = NULL;
	}
	
}

ID3D11Device *DX11Context::getDevice()
{
	return m_pDevice;
}

ID3D11DeviceContext *DX11Context::getDeviceContext()
{
	return m_pContext;
}

DX11FBOCache *DX11Context::getFBOCache()
{
	if (m_pFBOCache == NULL)
	{
		m_pFBOCache = new DX11FBOCache();
	}
	return m_pFBOCache;
}

DX11Texture *DX11Context::fetchTexture(const string &szFile, bool bGenMipmap)
{
	DX11Texture *pTex = NULL;

	EnterCriticalSection(&m_cs);
	GLTextureMap_::iterator itTex = m_mapTextures.find(szFile);
	if (itTex == m_mapTextures.end())
	{
		pTex = new DX11Texture();
		if (pTex->initTextureFromFile(szFile, bGenMipmap))
		{
			m_mapTextures.insert(make_pair(szFile, pTex));
		}
		else
		{
			delete pTex;
			pTex = NULL;
		}
	}
	else
	{
		pTex = itTex->second;
		pTex->ref();
	}
	LeaveCriticalSection(&m_cs);

	return pTex;
}

DX11Shader *DX11Context::fetchShader(const string &szShader, bool defaultCreate)
{
	//m_mapShaders
	DX11Shader *pShader = NULL;

	EnterCriticalSection(&m_cs);
	GLShaderMap_::iterator itShader = m_mapShaders.find(szShader);
	if (itShader == m_mapShaders.end())
	{
		if (defaultCreate)
		{
			pShader = new DX11Shader();
			if (pShader->initShaderWithString(szShader.c_str()))
			{
				m_mapShaders.insert(make_pair(szShader, pShader));
			}
			else
			{
				delete pShader;
				pShader = NULL;
			}
		}
		else
		{
			//return NULL;
		}
	}
	else
	{
		pShader = itShader->second;
		pShader->ref();
	}
	LeaveCriticalSection(&m_cs);

	return pShader;
}

void DX11Context::recordTexture(const string &szTexture, DX11Texture *pTex)
{
	if (pTex != NULL)
	{
		EnterCriticalSection(&m_cs);
		GLTextureMap_::iterator itTex = m_mapTextures.find(szTexture);
		if (itTex == m_mapTextures.end())
		{
			m_mapTextures.insert(make_pair(szTexture, pTex));
		}
		LeaveCriticalSection(&m_cs);
	}
}

void DX11Context::recordShader(const string &szShader, DX11Shader *pShader)
{
	if (pShader != NULL)
	{
		EnterCriticalSection(&m_cs);
		GLShaderMap_::iterator itShader = m_mapShaders.find(szShader);
		if (itShader == m_mapShaders.end())
		{
			m_mapShaders.insert(make_pair(szShader, pShader));
		}
		LeaveCriticalSection(&m_cs);
	}
}

void DX11Context::clearUnrefResource()
{
	EnterCriticalSection(&m_cs);
	{
		GLTextureMap_::iterator it = m_mapTextures.begin();
		while (it != m_mapTextures.end())
		{
			if (it->second->GetReferenceCount() <= 0)
			{
				it->second->destory();
				delete it->second;
				it = m_mapTextures.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	
	{
		GLShaderMap_::iterator it = m_mapShaders.begin();
		while (it != m_mapShaders.end())
		{
			if (it->second->GetReferenceCount() <= 0)
			{
				it->second->destory();
				delete it->second;
				it = m_mapShaders.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	
	LeaveCriticalSection(&m_cs);
}

ID3D11DepthStencilState *DX11Context::fetchDepthStencilState(bool enableDepthTest, bool enableDepthWrite)
{
	if (m_pDevice == NULL) { return NULL; }
	unsigned int id = 0;
	if (enableDepthTest)
	{
		id += 1;
	}
	if (enableDepthWrite)
	{
		id += 2;
	}

	DepthStencilStateMap_::iterator it = m_mapDepthStencialState.find(id);
	if (it == m_mapDepthStencialState.end())
	{
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		memset(&depth_stencil_desc, 0, sizeof(depth_stencil_desc));
		depth_stencil_desc.DepthEnable = enableDepthTest;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
		if (enableDepthWrite)
		{
			depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		}
		else
		{
			depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		}
		depth_stencil_desc.StencilEnable = FALSE;
		depth_stencil_desc.StencilReadMask = 0;
		depth_stencil_desc.StencilWriteMask = 0;

		ID3D11DepthStencilState *pDepthState = NULL;
		m_pDevice->CreateDepthStencilState(&depth_stencil_desc, &pDepthState);
		if (pDepthState != NULL)
		{
			pDepthState->AddRef();
			m_mapDepthStencialState.insert(make_pair(id, pDepthState));
		}
		
		return pDepthState;
	}
	else
	{
		it->second->AddRef();
		return it->second;
	}
}

ID3D11BlendState *DX11Context::fetchBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer)
{
	if (m_pDevice == NULL) { return NULL; }
	unsigned int id = 0;
	if (bBlend)
	{
		id += 1;
	}
	if (bBlendAlpha)
	{
		id += 2;
	}
	if (writeBuffer)
	{
		id += 4;
	}

	//BlendStateMap_ m_mapBlendState

	BlendStateMap_::iterator it = m_mapBlendState.find(id);
	if (it == m_mapBlendState.end())
	{
		D3D11_BLEND_DESC blend_desc;
		memset(&blend_desc, 0, sizeof(blend_desc));
		blend_desc.AlphaToCoverageEnable = false;
		blend_desc.IndependentBlendEnable = false;
		blend_desc.RenderTarget[0].BlendEnable = bBlend;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		if (bBlendAlpha)
		{
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		}
		else
		{
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		}

		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

		if (writeBuffer)
		{
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		else
		{
			blend_desc.RenderTarget[0].RenderTargetWriteMask = 0;
		}

		ID3D11BlendState *pBlendState = NULL;
		DevicePtr->CreateBlendState(&blend_desc, &pBlendState);
		if (pBlendState != NULL)
		{
			pBlendState->AddRef();
			m_mapBlendState.insert(make_pair(id, pBlendState));
		}

		return pBlendState;
	}
	else
	{
		it->second->AddRef();
		return it->second;
	}
}

ID3D11RasterizerState *DX11Context::fetchRasterizerState(D3D11_CULL_MODE cullMode)
{
	if (m_pDevice == NULL) { return NULL; }
	unsigned int id = 0;
	id = cullMode;

	RasterizerStateMap_::iterator it = m_mapRasterizerState.find(id);
	if (it == m_mapRasterizerState.end())
	{
		D3D11_RASTERIZER_DESC rasterizer_desc;
		memset(&rasterizer_desc, 0, sizeof(D3D11_RASTERIZER_DESC));
		rasterizer_desc.CullMode = cullMode;
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.AntialiasedLineEnable = true;
		rasterizer_desc.MultisampleEnable = true;
		ID3D11RasterizerState *pRasterizerState = NULL;
		DevicePtr->CreateRasterizerState(&rasterizer_desc, &pRasterizerState);
		if (pRasterizerState != NULL)
		{
			pRasterizerState->AddRef();
			m_mapRasterizerState.insert(make_pair(id, pRasterizerState));
		}

		return pRasterizerState;
	}
	else
	{
		it->second->AddRef();
		return it->second;
	}
}

void DX11Context::setCullMode(D3D11_CULL_MODE cullMode)
{
	ID3D11RasterizerState *pRSCull = fetchRasterizerState(cullMode);
	if (pRSCull != NULL)
	{
		if (m_pContext != NULL) { m_pContext->RSSetState(pRSCull);}
		pRSCull->Release();
	}
}