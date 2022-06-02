#include "DX11Shader.h"
#include <d3dx11.h>  
#include <d3dcompiler.h>  
#include <xnamath.h>  
#include <iostream>
#include <locale>
#include <codecvt>
#include <assert.h>
#include "DX11Context.h"
#include "Toolbox/Helper.h"
#include "Toolbox/string_util.h"

using namespace std;

DX11Shader::DX11Shader()
{
	m_pVertexShader = NULL;
	m_pPixelShader = NULL;

	m_pVertexLayout = NULL;
	m_pArrtibute = NULL;
	m_nAttribute = 0;
}


DX11Shader::~DX11Shader()
{
	destory();
}

//--------------------------------------------------------------------------------------  
// Helper for compiling shaders with D3DX11  
//--------------------------------------------------------------------------------------  
HRESULT DX11Shader::CompileShaderFromFile(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS| D3DCOMPILE_SKIP_OPTIMIZATION;
#if defined( DEBUG ) || defined( _DEBUG )  
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.  
	// Setting this flag improves the shader debugging experience, but still allows   
	// the shaders to be optimized and to run exactly the way they will run in   
	// the release configuration of this program.  
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif  
	std::wstring wPath = core::u8_ucs2(szFileName);

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFileW(wPath.c_str(), NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			cout << (char*)pErrorBlob->GetBufferPointer() << endl;
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT DX11Shader::CompileShaderFromString(const char* szShader, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromMemory(
		szShader,
		strlen(szShader),
		NULL,
		NULL,
		NULL,
		szEntryPoint,
		szShaderModel,
		dwShaderFlags,
		0,
		NULL,
		ppBlobOut,
		&pErrorBlob,
		NULL
	);

	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)cout << (char*)pErrorBlob->GetBufferPointer() << endl;
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();
	return S_OK;
}


bool DX11Shader::initShaderWithFile(const char *szShader)
{
	assert(szShader != NULL);

	HRESULT hr = S_OK;
	ID3DBlob *pVSBlob = NULL;
	hr = CompileShaderFromFile((char *)szShader, "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		cout << "VS compile error!" << endl;
		return false;
	}

	hr = DevicePtr->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}
	initInputLayout(pVSBlob);
	pVSBlob->Release();


	ID3DBlob *pPSBlob = NULL;
	hr = S_OK;
	hr = CompileShaderFromFile((char *)szShader, "PS", "ps_4_0", &pPSBlob);
 	if (FAILED(hr))
	{
		cout << "compile shader failed!" << endl;
		return false;
	}

	hr = DevicePtr->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();
	
	return hr == S_OK;
}

bool DX11Shader::initShaderWithString(const char *szShader)
{
	assert(szShader != NULL);

	HRESULT hr = S_OK;
	ID3DBlob *pVSBlob = NULL;
	hr = CompileShaderFromString(szShader, "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		cout << "VS compile error!" << endl;
		return false;
	}

	hr = DevicePtr->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}
	initInputLayout(pVSBlob);
	pVSBlob->Release();


	ID3DBlob *pPSBlob = NULL;
	hr = S_OK;
	hr = CompileShaderFromString(szShader, "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		cout << "compile shader failed!" << endl;
		return false;
	}

	hr = DevicePtr->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();

	return hr == S_OK;
}


bool DX11Shader::initInputLayout(ID3DBlob *pVSBlob)
{
	if (m_Separate)
	{
		HRESULT hr = S_OK;
		if (m_nAttribute == 0)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			UINT numElements = ARRAYSIZE(layout);
			hr = DevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
		}
		else
		{
			string arrSemantic[] = { "POSITION", "COLOR", "TEXCOORD", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3", "NORMAL", "BLENDWEIGHT", "BLENDINDICES", "TANGENT", "BINORMAL" };
			DXGI_FORMAT arrFormat[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R16G16B16A16_UINT};
			D3D11_INPUT_ELEMENT_DESC layout[10];
			for (int i = 0; i < m_nAttribute; ++i)
			{
				layout[i] = { arrSemantic[m_pArrtibute[i].m_data].c_str(), 0, arrFormat[m_pArrtibute[i].m_class], (UINT)i, 0, D3D11_INPUT_PER_VERTEX_DATA , 0 };

				if (m_pArrtibute[i].m_data == VERTEX_ATTRIB_TEX_COORD1)
				{
					layout[i] = { "TEXCOORD", 1, arrFormat[m_pArrtibute[i].m_class], (UINT)i, 0, D3D11_INPUT_PER_VERTEX_DATA , 0 };
				}
				
			}
			hr = DevicePtr->CreateInputLayout(layout, m_nAttribute, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
		}
		if (FAILED(hr))
		{
			cout << "initInputLayout error!" << endl;
			return false;
		}
		return (hr == S_OK);
	}
	else
	{
		HRESULT hr = S_OK;
		if (m_nAttribute == 0)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			UINT numElements = ARRAYSIZE(layout);
			hr = DevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
		}
		else
		{
			string arrSemantic[] = { "POSITION", "COLOR", "TEXCOORD", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3", "NORMAL", "BLENDWEIGHT", "BLENDINDICES", "TANGENT", "BINORMAL" };
			DXGI_FORMAT arrFormat[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };
			D3D11_INPUT_ELEMENT_DESC layout[10];
			unsigned int step = 0;
			for (int i = 0; i < m_nAttribute; ++i)
			{
				layout[i] = { arrSemantic[m_pArrtibute[i].m_data].c_str(), 0, arrFormat[m_pArrtibute[i].m_class], 0, step, D3D11_INPUT_PER_VERTEX_DATA , 0 };

				if (m_pArrtibute[i].m_data == VERTEX_ATTRIB_TEX_COORD1)
				{
					layout[i] = { "TEXCOORD", 1, arrFormat[m_pArrtibute[i].m_class], 0, step, D3D11_INPUT_PER_VERTEX_DATA , 0 };
				}

				step += sizeof(float)*(m_pArrtibute[i].m_class + 1);
			}
			hr = DevicePtr->CreateInputLayout(layout, m_nAttribute, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
		}
		if (FAILED(hr))
		{
			cout << "initInputLayout error!" << endl;
			return false;
		}
		return (hr == S_OK);
	}

}

void DX11Shader::useShader()
{
	if (m_pVertexShader != NULL && m_pPixelShader != NULL && m_pVertexLayout != NULL)
	{
		//¶¥µã
		DeviceContextPtr->VSSetShader(m_pVertexShader, NULL, 0);
		DeviceContextPtr->IASetInputLayout(m_pVertexLayout);

		//ÏñËØ
		DeviceContextPtr->PSSetShader(m_pPixelShader, NULL, 0);
	}
}

void DX11Shader::destory()
{
	SAFERALEASE(m_pVertexShader);
	SAFERALEASE(m_pPixelShader);
	SAFERALEASE(m_pVertexLayout);
}

void DX11Shader::SetAttribute(CCVetexAttribute * pAttri, int nAttri)
{
	m_pArrtibute = pAttri;
	m_nAttribute = nAttri;
}
