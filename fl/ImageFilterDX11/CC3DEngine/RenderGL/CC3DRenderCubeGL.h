#ifndef _H_CC3D_RENDERER__CUBE_H_
#define _H_CC3D_RENDERER__CUBE_H_

#include "GL/CCProgram.h"
#include"GL/CCFrameBuffer.h"
#include "Common/CC3DUtils.h"
#include "Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"

BEGIN_SHADER_STRUCT(CubeMap,0)
	DECLARE_PARAM(glm::mat4, projection)
	BEGIN_STRUCT_CONSTRUCT(CubeMap)
		IMPLEMENT_PARAM("projection", UniformType::MAT4)
		INIT_TEXTURE_INDEX("equirectangularMap", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(IBL_Irradiance, 0)
	DECLARE_PARAM(glm::mat4, projection)
	DECLARE_PARAM(glm::mat4, view)
	BEGIN_STRUCT_CONSTRUCT(IBL_Irradiance)
		IMPLEMENT_PARAM("projection", UniformType::MAT4)
		IMPLEMENT_PARAM("view", UniformType::MAT4)
		INIT_TEXTURE_INDEX("environmentMap", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(IBL_Prefilter, 0)
	DECLARE_PARAM(glm::mat4, projection)
	DECLARE_PARAM(glm::mat4, view)
	DECLARE_PARAM_VALUE(float, roughness,1.f)
	BEGIN_STRUCT_CONSTRUCT(IBL_Prefilter)
		IMPLEMENT_PARAM("projection", UniformType::MAT4)
		IMPLEMENT_PARAM("view", UniformType::MAT4)
		IMPLEMENT_PARAM("roughness", UniformType::FLOAT)
		INIT_TEXTURE_INDEX("environmentMap", 0)
		INIT_TEXTURE_INDEX("Hammersley", 1)
	END_SHADER_STRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(IBL_Brdf, 0)
	BEGIN_STRUCT_CONSTRUCT(IBL_Brdf)
		INIT_TEXTURE_INDEX("Hammersley", 0)
	END_SHADER_STRUCT
END_SHADER_STRUCT
;
class CC3DRenderCubeGL
{
public:
	CC3DRenderCubeGL();
	~CC3DRenderCubeGL();
	
	virtual void Render();
	void renderCube();
	void setHDR(std::string  filePath);
	void SetShaderResource(std::string path);
	void setHDRData(float *pData, int &nWidth, int &nHeight);

	std::shared_ptr<CC3DCubeMapRHI> EnvCube;
	std::shared_ptr<CC3DTextureRHI> HdrCube;
	std::shared_ptr<CC3DCubeMapRHI> PreCube;
	std::shared_ptr<CC3DCubeMapRHI> IrrCube;
	std::shared_ptr<CC3DRenderTargetRHI> Blut;

private:
	virtual void Initialize();

	std::string m_resourcePath;
	std::string HDRFilePath;

	std::shared_ptr<CC3DVertexBuffer> m_CubeVertexBuffer;
	std::shared_ptr<CC3DTextureRHI> m_Ham;
	std::shared_ptr<CC3DVertexBuffer> m_QuadVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> m_QuadIndexBuffer;

	//渲染使用的program
	std::shared_ptr<CCProgram> m_program;
	std::shared_ptr<CCProgram> m_programBlut;
	std::shared_ptr<CCProgram> m_programPre;
	std::shared_ptr<CCProgram> m_programIRR;

	DECLARE_SHADER_STRUCT_MEMBER(CubeMap)
	DECLARE_SHADER_STRUCT_MEMBER(IBL_Irradiance)
	DECLARE_SHADER_STRUCT_MEMBER(IBL_Prefilter)
	DECLARE_SHADER_STRUCT_MEMBER(IBL_Brdf)
};

#endif //_H_CC3D_RENDERER__CUBE_H_