#pragma once
#include "../3d/CCMesh.h"

NS_CC_BEGIN
class CCShaderCache
{
public:
	static CCShaderCache* shareInst();

	static void destoryInst();

	DX11Shader *fetchShader(MeshVertexAttrib *attrib, int nSize, bool lighting = true,bool bNPR = false);
protected:
	CCShaderCache();
	~CCShaderCache();

	void loadDefaultShaders();

	DX11Shader *m_pDiffuseShader;
	DX11Shader *m_pDiffuseShaderSkin;
	DX11Shader *m_pDiffuseLitShader;
	DX11Shader *m_pDiffuseLitShaderSkin;
	DX11Shader *m_pDiffuseShaderSkin_NPR;

private:
	static CCShaderCache *s_pInst;
};
NS_CC_END
