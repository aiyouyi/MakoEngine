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
#include "string_util.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)


typedef HRESULT(WINAPI*
D3DCompileFromFile47)(LPCWSTR pFileName,
	 CONST D3D_SHADER_MACRO* pDefines,
	 ID3DInclude* pInclude,
	 LPCSTR pEntrypoint,
	 LPCSTR pTarget,
	 UINT Flags1,
	 UINT Flags2,
	 ID3DBlob** ppCode,
	 ID3DBlob** ppErrorMsgs);

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
	hr = D3DX11CompileFromFileW(wPath.c_str(), nullptr, nullptr, szEntryPoint, szShaderModel,
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

HRESULT DX11Shader::CompileShaderFromFile47(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* pDefines)
{

	if (!m_D3DCompile47.handle())
	{
		if (core::error_ok != m_D3DCompile47.load("D3DCompiler_47.dll"))
		{
			return CompileShaderFromFile(szFileName,szEntryPoint,szShaderModel,ppBlobOut);
		}
	}

	D3DCompileFromFile47 D3DCompile47 = (D3DCompileFromFile47)m_D3DCompile47.proc("D3DCompileFromFile");
	if (!D3DCompile47)
	{
		return CompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, ppBlobOut);
	}

	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION;
#if defined( DEBUG ) || defined( _DEBUG )  
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.  
	// Setting this flag improves the shader debugging experience, but still allows   
	// the shaders to be optimized and to run exactly the way they will run in   
	// the release configuration of this program.  
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif  

	std::wstring wPath = core::u8_ucs2(szFileName);
	ID3DBlob* pErrorBlob = nullptr;

	hr =  D3DCompile47(wPath.c_str(), pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
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

	return initShaderWithFile(szShader, "VS", "PS", 4);
}

bool DX11Shader::initShaderWithFile(const std::string& fileName, const std::string& VSMain, const std::string& PSMain, int level)
{

	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(fileName.c_str(), VSMain.c_str(),core::format_printf("vs_%d_0",level).c_str(), &pVSBlob);
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


	ID3DBlob* pPSBlob = NULL;
	hr = S_OK;
	hr = CompileShaderFromFile(fileName.c_str(), PSMain.c_str(), core::format_printf("ps_%d_0", level).c_str(), &pPSBlob);
	if (FAILED(hr))
	{
		cout << "compile shader failed!" << endl;
		return false;
	}

	hr = DevicePtr->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader);
	pPSBlob->Release();

	return hr == S_OK;
}

bool DX11Shader::initGeometryWithStreamOutput(const std::string& fileName, const std::string& VSMain, const std::string& GSMain, const std::vector< D3D11SOLayout>& SOLayouts)
{

	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(fileName.c_str(), VSMain.c_str(), "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		cout << "VS compile error!" << endl;
		return false;
	}

	hr = DevicePtr->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}
	initInputLayout(pVSBlob);
	pVSBlob->Release();

	ID3DBlob* pGSBlob = nullptr;
	hr = S_OK;
	hr = CompileShaderFromFile(fileName.c_str(), GSMain.c_str(), "gs_5_0", &pGSBlob);
	if (FAILED(hr))
	{
		cout << "compile shader failed!" << endl;
		return false;
	}

	std::vector< D3D11_SO_DECLARATION_ENTRY> outputLayout;
	outputLayout.resize(SOLayouts.size());
	uint32_t stride = 0;
	for (size_t index = 0; index < SOLayouts.size(); ++index)
	{
		outputLayout[index].ComponentCount = SOLayouts[index].ComponentCount;
		outputLayout[index].OutputSlot = SOLayouts[index].OutputSlot;
		outputLayout[index].SemanticIndex = SOLayouts[index].SemanticIndex;
		outputLayout[index].SemanticName = SOLayouts[index].SemanticName.c_str();
		outputLayout[index].StartComponent = SOLayouts[index].StartComponent;
		outputLayout[index].Stream = SOLayouts[index].Stream;
		stride += outputLayout[index].ComponentCount * 4;
	}

	uint32_t strides[] = { stride };
	hr = DevicePtr->CreateGeometryShaderWithStreamOutput(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(),
		outputLayout.data(), outputLayout.size(), strides, 1, D3D11_SO_NO_RASTERIZED_STREAM,
		nullptr, &m_GeometryShader);

	pGSBlob->Release();

	return hr == S_OK;
}

bool DX11Shader::initGeometryWithFile(const std::string& fileName, const std::string& VSMain, const std::string& GSMain, const std::string& PSMain )
{
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(fileName.c_str(), VSMain.c_str(), "vs_5_0", &pVSBlob);
	if (FAILED(hr))
	{
		cout << "VS compile error!" << endl;
		return false;
	}

	hr = DevicePtr->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return false;
	}
	initInputLayout(pVSBlob);
	pVSBlob->Release();

	ID3DBlob* pGSBlob = nullptr;
	hr = CompileShaderFromFile(fileName.c_str(), GSMain.c_str(), "gs_5_0", &pGSBlob);
	if (FAILED(hr))
	{
		cout << "compile shader failed!" << endl;
		return false;
	}
	DevicePtr->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_GeometryShader);

	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(fileName.c_str(), PSMain.c_str(), "ps_5_0", &pPSBlob);
	if (FAILED(hr))
	{
		cout << "compile shader failed!" << endl;
		return false;
	}

	hr = DevicePtr->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
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


bool DX11Shader::initVSShader(const std::string& fileName, const std::string& VSMain, bool InitInputLayout, const D3D_SHADER_MACRO* pDefines)
{
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile47(fileName.c_str(), VSMain.c_str(), "vs_5_0", &pVSBlob, pDefines);
	if (FAILED(hr))
	{
		cout << "VS compile error!" << endl;
		return false;
	}

	hr = DevicePtr->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);

	if (InitInputLayout)
	{
		initInputLayout(pVSBlob);
	}

	pVSBlob->Release();

	return hr == S_OK;
}

bool DX11Shader::initPSShader(const std::string& fileName, const std::string& PSMain, const D3D_SHADER_MACRO* pDefines)
{
	HRESULT hr = S_OK;
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile47(fileName.c_str(), PSMain.c_str(), "ps_5_0", &pPSBlob, pDefines);
	if (FAILED(hr))
	{
		cout << "PS compile error!" << endl;
		return false;
	}

	hr = DevicePtr->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);

	pPSBlob->Release();
	return hr == S_OK;
}

bool DX11Shader::initGeometryShader(const std::string& fileName, const std::string& GSMain, const D3D_SHADER_MACRO* pDefines)
{
	ID3DBlob* pGSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile47(fileName.c_str(), GSMain.c_str(), "gs_5_0", &pGSBlob, pDefines);
	if (FAILED(hr))
	{
		cout << "GS shader failed!" << endl;
		return false;
	}
	hr = DevicePtr->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &m_GeometryShader);

	pGSBlob->Release();
	return hr == S_OK;
}

bool DX11Shader::initComputeShader(const std::string& fileName, const std::string& GSMain, const D3D_SHADER_MACRO* pDefines)
{
	ID3DBlob* pCSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile47(fileName.c_str(), GSMain.c_str(), "cs_5_0", &pCSBlob, pDefines);
	if (FAILED(hr))
	{
		cout << "CS shader failed!" << endl;
		return false;
	}
	hr = DevicePtr->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &m_ComouteShader);

	pCSBlob->Release();
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

			uint32_t numElements = ARRAYSIZE(layout);
			hr = DevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
		}
		else
		{
			std::string arrSemantic[] = { "POSITION", "COLOR", "TEXCOORD", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3", "NORMAL", "BLENDWEIGHT", "BLENDINDICES", "TANGENT", "BINORMAL","VELOCITY","SIZE","AGE","TYPE"};
			DXGI_FORMAT arrFormat[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R16G16B16A16_UINT,DXGI_FORMAT_R32G32B32_FLOAT,DXGI_FORMAT_R32G32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_UINT };
			D3D11_INPUT_ELEMENT_DESC layout[10];
			for (int i = 0; i < m_nAttribute; ++i)
			{
				layout[i] = { arrSemantic[m_pArrtibute[i].m_data].c_str(), 0, arrFormat[m_pArrtibute[i].m_class], (uint32_t)i, 0, D3D11_INPUT_PER_VERTEX_DATA , 0 };

				if (m_pArrtibute[i].m_data == VERTEX_ATTRIB_TEX_COORD1)
				{
					layout[i] = { "TEXCOORD", 1, arrFormat[m_pArrtibute[i].m_class], (uint32_t)i, 0, D3D11_INPUT_PER_VERTEX_DATA , 0 };
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
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			uint32_t numElements = ARRAYSIZE(layout);
			hr = DevicePtr->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
		}
		else
		{
			std::string arrSemantic[] = { "POSITION", "COLOR", "TEXCOORD", "TEXCOORD1", "TEXCOORD2", "TEXCOORD3", "NORMAL", "BLENDWEIGHT", "BLENDINDICES", "TANGENT", "BINORMAL","VELOCITY","SIZE","AGE","TYPE" };
			DXGI_FORMAT arrFormat[] = { DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R16G16B16A16_UINT,DXGI_FORMAT_R32G32B32_FLOAT,DXGI_FORMAT_R32G32_FLOAT,DXGI_FORMAT_R32_FLOAT,DXGI_FORMAT_R32_UINT };
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
	if (m_pVertexShader)
	{
		DeviceContextPtr->VSSetShader(m_pVertexShader, nullptr, 0);
	}

	if (m_pVertexLayout)
	{
		DeviceContextPtr->IASetInputLayout(m_pVertexLayout);
	}

	if (m_pPixelShader)
	{
		DeviceContextPtr->PSSetShader(m_pPixelShader, nullptr, 0);
	}

	if (m_GeometryShader)
	{
		DeviceContextPtr->GSSetShader(m_GeometryShader, nullptr, 0);
	}
	else
	{
		DeviceContextPtr->GSSetShader(nullptr, nullptr, 0);
	}

	if (m_ComouteShader)
	{
		DeviceContextPtr->CSSetShader(m_ComouteShader, nullptr, 0);
	}
	else
	{
		DeviceContextPtr->CSSetShader(nullptr, nullptr, 0);
	}
}

void DX11Shader::destory()
{
	SAFERALEASE(m_pVertexShader);
	SAFERALEASE(m_pPixelShader);
	SAFERALEASE(m_pVertexLayout);
	SAFERALEASE(m_GeometryShader);
}

void DX11Shader::SetAttribute(CCVetexAttribute * pAttri, int nAttri)
{
	m_pArrtibute = pAttri;
	m_nAttribute = nAttri;
}

void DX11Shader::SetSeparate(bool bSeparate)
{
	m_Separate = bSeparate;
}
