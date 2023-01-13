#ifndef _H_CC3D_RENDERER__CUBE_H_
#define _H_CC3D_RENDERER__CUBE_H_

#include "ToolBox/DXUtils/DX11CubeMap.h"

#include "ToolBox/DXUtils/DX11Texture.h"
#include "ToolBox/DXUtils/DX11FBO.h"
#include "ToolBox/DXUtils/DX11Shader.h"
#include "Common/CC3DUtils.h"
#include "Toolbox/Render/CubeMapRHI.h"
#include "Toolbox/Render/DynamicRHI.h"

struct IBLConstantBuffer
{

	glm::mat4 view;
	glm::mat4 projection;
	Vector4 Roughness;
};


class CC3DRenderCube
{
public:
	CC3DRenderCube();
	~CC3DRenderCube();
	
	virtual void Render();
	void renderCube();
	void setHDR(const std::string&  filePath);
	void SetShaderResource(const std::string& path);
	void setHDRData(float *pData, int nWidth, int nHeight);

	std::shared_ptr<CC3DCubeMapRHI> m_envCube;
	std::shared_ptr<CC3DCubeMapRHI> m_PreCube;
	std::shared_ptr<CC3DCubeMapRHI> m_IrrCube;

	std::shared_ptr<CC3DTextureRHI> m_hdrCube;
	std::shared_ptr<CC3DRenderTargetRHI> m_Blut;
	std::shared_ptr<CC3DTextureRHI> m_BlutTexture;

private:
	virtual void Initialize();

	std::shared_ptr<CC3DVertexBuffer> m_cubeVAO;

	std::string m_resourcePath;
	std::string HDRFilePath;


	//渲染使用的program
	DX11Shader* m_program;
	DX11Shader* m_programBlut;
	DX11Shader* m_programPre;
	DX11Shader* m_programIRR;

	IBLConstantBuffer m_IBLConstantBuffer;
	std::shared_ptr<CC3DConstantBuffer> m_IBLCB;
	std::shared_ptr<CC3DVertexBuffer> m_QuadVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> m_QuadIndexBuffer;
};

#endif //_H_CC3D_RENDERER__CUBE_H_