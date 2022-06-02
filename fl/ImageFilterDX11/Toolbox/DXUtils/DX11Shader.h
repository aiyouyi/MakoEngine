#pragma once
#include <d3d11.h>
#include "IReferenceCounted.h"

enum CCVetexData
{
	/**Index 0 will be used as Position.*/
	VERTEX_ATTRIB_POSITION = 0,
	/**Index 1 will be used as Color.*/
	VERTEX_ATTRIB_COLOR,
	/**Index 2 will be used as Tex coord unit 0.*/
	VERTEX_ATTRIB_TEX_COORD,
	/**Index 3 will be used as Tex coord unit 1.*/
	VERTEX_ATTRIB_TEX_COORD1,
	/**Index 4 will be used as Tex coord unit 2.*/
	VERTEX_ATTRIB_TEX_COORD2,
	/**Index 5 will be used as Tex coord unit 3.*/
	VERTEX_ATTRIB_TEX_COORD3,
	/**Index 6 will be used as Normal.*/
	VERTEX_ATTRIB_NORMAL,
	/**Index 7 will be used as Blend weight for hardware skin.*/
	VERTEX_ATTRIB_BLEND_WEIGHT,
	/**Index 8 will be used as Blend index.*/
	VERTEX_ATTRIB_BLEND_INDEX,
	/**Index 9 will be used as tangent.*/
	VERTEX_ATTRIB_TANGENT,
	/**Index 10 will be used as Binormal.*/
	VERTEX_ATTRIB_BINORMAL,
	VERTEX_ATTRIB_MAX,

	// backward compatibility
	VERTEX_ATTRIB_TEX_COORDS = VERTEX_ATTRIB_TEX_COORD,
};
enum CCVetexClass
{
	FLOAT_C1 = 0,
	FLOAT_C2,
	FLOAT_C3,
	FLOAT_C4,
	SHORT_C4,
};

struct CCVetexAttribute
{
	CCVetexData m_data;
	CCVetexClass m_class;
};

class DX11IMAGEFILTER_EXPORTS_CLASS DX11Shader: public IReferenceCounted
{
public:
	DX11Shader();
	virtual ~DX11Shader();
	
	bool initShaderWithFile(const char *szShader);
	bool initShaderWithString(const char *szShader);
	virtual bool initInputLayout(ID3DBlob *pVSBlob);

	void useShader();
	void destory();

	void SetAttribute(CCVetexAttribute *pAttri, int nAttri);

	bool m_Separate = false;
protected:
	HRESULT CompileShaderFromFile(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT CompileShaderFromString(const char* szShader, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
protected:
	ID3D11VertexShader*     m_pVertexShader;
	ID3D11PixelShader*      m_pPixelShader;

	ID3D11InputLayout*      m_pVertexLayout;

	CCVetexAttribute *m_pArrtibute;
	int m_nAttribute;
};

