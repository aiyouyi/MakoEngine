#pragma once
#include <d3d11.h>
#include "IReferenceCounted.h"
#include "Toolbox/Render/ShaderRHI.h"
#include "Toolbox/win/dll.h"

class DX11IMAGEFILTER_EXPORTS_CLASS DX11Shader: public IReferenceCounted
{
public:
	DX11Shader();
	virtual ~DX11Shader();
	
	bool initShaderWithFile(const char *szShader);
	bool initShaderWithFile(const std::string& fileName,const std::string& VSMain,const std::string& PSMain,int level = 4);
	bool initGeometryWithStreamOutput(const std::string& fileName, const std::string& VSMain, const std::string& GSMain, const std::vector< D3D11SOLayout>& SOLayouts);
	bool initGeometryWithFile(const std::string& fileName, const std::string& VSMain, const std::string& GSMain, const std::string& PSMain);
	bool initShaderWithString(const char *szShader);

	bool initVSShader(const std::string& fileName, const std::string& VSMain, bool InitInputLayout , const D3D_SHADER_MACRO* pDefines);
	bool initPSShader(const std::string& fileName, const std::string& PSMain, const D3D_SHADER_MACRO* pDefines);
	bool initGeometryShader(const std::string& fileName, const std::string& GSMain, const D3D_SHADER_MACRO* pDefines);
	bool initComputeShader(const std::string& fileName, const std::string& GSMain, const D3D_SHADER_MACRO* pDefines);

	void useShader();
	void destory();

	void SetAttribute(CCVetexAttribute *pAttri, int nAttri);
	void SetSeparate(bool bSeparate);

private:
	bool initInputLayout(ID3DBlob* pVSBlob);
protected:
	HRESULT CompileShaderFromFile(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT CompileShaderFromFile47(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const D3D_SHADER_MACRO* pDefines = nullptr);
	HRESULT CompileShaderFromString(const char* szShader, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
protected:
	ID3D11VertexShader*     m_pVertexShader = nullptr;
	ID3D11PixelShader*      m_pPixelShader = nullptr;
	ID3D11GeometryShader*   m_GeometryShader = nullptr;
	ID3D11ComputeShader*	m_ComouteShader = nullptr;
	ID3D11InputLayout*      m_pVertexLayout = nullptr;

	CCVetexAttribute *m_pArrtibute = nullptr;
	int m_nAttribute;
	bool m_Separate = false;

	win32::dll m_D3DCompile47;
};

