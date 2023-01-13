#include "CC3DRenderCubeGL.h"
#include "RenderState/PiplelineState.h"
#include "Render/CubeMapRHI.h"
#include "Render/RenderTargetRHI.h"

static float RadicalInverse_VdC(unsigned int bits)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
	bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
	bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
	return (bits * 2.3283064365386963e-10);// / 0x100000000
	//0.0000152588
}
// ----------------------------------------------------------------------------
static float Hammersley(int i, int N)
{
	return RadicalInverse_VdC(i);
}


static void CalHammersley(float* pHammersley)
{
	for (int i = 0; i < 1024; i++)
	{
		pHammersley[i * 3] = i * 1.0 / 1024;
		pHammersley[i * 3 + 1] = RadicalInverse_VdC(i);
		pHammersley[i * 3 + 2] = 0.0;
	}
}

CC3DRenderCubeGL::CC3DRenderCubeGL()
{
}

CC3DRenderCubeGL::~CC3DRenderCubeGL()
{

}

void CC3DRenderCubeGL::Initialize()
{

	EnvCube = GetDynamicRHI()->CreateCubeMap(512, 512, 0,false,true);
	IrrCube = GetDynamicRHI()->CreateCubeMap(32, 32, 0, false, false);
	PreCube = GetDynamicRHI()->CreateCubeMap(128, 128, 0, false, true);
	GetDynamicRHI()->GenerateMips(PreCube);
	Blut = GetDynamicRHI()->CreateRenderTarget(128, 128, false, nullptr, CC3DTextureRHI::SFT_R16G16B16F);


	std::shared_ptr<float> pHammersley(new float[3 * 1024], [](void* p) {delete[]p; });

	CalHammersley(pHammersley.get());
	m_Ham = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R32G32B32F, 0, 1024, 1, pHammersley.get());

	Vector3 arrCoords[3] = { Vector3(-1, -1, 1), Vector3(3, -1, 1), Vector3(-1, 3, 1) };

	uint16 index[] = {
		0, 1, 2
	};

	m_QuadVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, sizeof(arrCoords)/sizeof(arrCoords[0]), 3);
	m_QuadIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 1);
}


void CC3DRenderCubeGL::Render()
{

	//glEnable(GL_DEPTH_TEST);
	// set depth function to less than AND equal for skybox depth trick.
	//glDepthFunc(GL_LEQUAL);
	// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	
#ifdef PLATFORM_WINDOWS
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +X
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)), // -X
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),	  // +Y
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), // -Y
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),  // +Z
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))  // -Z
	};

	m_program->Use();
	GET_SHADER_STRUCT_MEMBER(CubeMap).SetTexture("equirectangularMap", HdrCube);
	GET_SHADER_STRUCT_MEMBER(CubeMap).SetMatrix4Parameter("projection", &captureProjection[0][0], false, 1);

	for (uint32_t i = 0; i < 6; ++i)
	{
		EnvCube->SetRenderTarget(i, 0);
		
		m_program->SetUniformMatrix4fv("view", &captureViews[i][0][0], false, 1);
		EnvCube->ClearDepthBuffer();
		renderCube();
		EnvCube->UnBindRenderTarget();
	}
	GetDynamicRHI()->GenerateMips(EnvCube);

	// -----------------------------------------------------------------------------
	m_programIRR->Use();
	GET_SHADER_STRUCT_MEMBER(IBL_Irradiance).SetTexture("environmentMap", EnvCube);
	GET_SHADER_STRUCT_MEMBER(IBL_Irradiance).SetMatrix4Parameter("projection", &captureProjection[0][0], false, 1);

	for (unsigned int i = 0; i < 6; ++i)
	{
		IrrCube->SetRenderTarget(i, 0);
		GET_SHADER_STRUCT_MEMBER(IBL_Irradiance).SetMatrix4Parameter("view", &captureViews[i][0][0], false, 1);
		IrrCube->ClearDepthBuffer();
		renderCube();
		IrrCube->UnBindRenderTarget();
	}


	m_programPre->Use();
	GET_SHADER_STRUCT_MEMBER(IBL_Prefilter).SetTexture("environmentMap", EnvCube);
	GET_SHADER_STRUCT_MEMBER(IBL_Prefilter).SetTexture("Hammersley", m_Ham);
	GET_SHADER_STRUCT_MEMBER(IBL_Prefilter).SetParameter("projection", captureProjection);

	unsigned int maxMipLevels = 8;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		GET_SHADER_STRUCT_MEMBER(IBL_Prefilter).SetParameter("roughness", roughness);

		for (unsigned int i = 0; i < 6; ++i)
		{
			GET_SHADER_STRUCT_MEMBER(IBL_Prefilter).SetParameter("view", captureViews[i]);
			PreCube->SetRenderTarget(i, mip);
			PreCube->ClearDepthBuffer();
			renderCube();
			PreCube->UnBindRenderTarget();
		}
	}

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

	m_programBlut->Use();

	GET_SHADER_STRUCT_MEMBER(IBL_Brdf).SetTexture("Hammersley", m_Ham);

	Blut->Bind();
	Blut->Clear(0, 0, 0, 1);
	GetDynamicRHI()->DrawPrimitive(m_QuadVertexBuffer, m_QuadIndexBuffer);
	Blut->UnBind();
}


void CC3DRenderCubeGL::renderCube()
{
	// initialize (if necessary)
	if (m_CubeVertexBuffer == nullptr)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top-right         
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-right     
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
			-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f  // bottom-left        
		};
		int pArrange[] = { 3,3,2 };
		m_CubeVertexBuffer = GetDynamicRHI()->CreateVertexBuffer(vertices, 36, pArrange, 3, 8);
	}

	GetDynamicRHI()->DrawPrimitiveTriangles(m_CubeVertexBuffer);
}

void CC3DRenderCubeGL::setHDR(std::string filePath)
{
	HDRFilePath = filePath;
	int width, height, nrComponents;
	float *data = ccLoadImagef(HDRFilePath.c_str(), &width, &height, &nrComponents,4);

	if (data)
	{
		HdrCube = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R16G16B16F, 0, width, height, data, 0, false, false);
		free(data);
	}

}

void CC3DRenderCubeGL::setHDRData(float *pData, int &nWidth, int &nHeight)
{
	if (pData)
	{
		HdrCube = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R16G16B16F, 0, nWidth, nHeight, pData, 0, false, false);
	}
	else
	{
		LOGE("Failed to load HDR image.");
	}
}

void CC3DRenderCubeGL::SetShaderResource(std::string path)
{
	m_resourcePath = path;
	if (m_program != NULL)
	{
		return;
	}
	m_program = std::make_shared<CCProgram>();
	std::string  vspath = m_resourcePath + "/Shader/3D/cubemap.vs";
	std::string  fspath = m_resourcePath + "/Shader/3D/equirectangular_to_cubemap.fs";
	m_program->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
	GET_SHADER_STRUCT_MEMBER(CubeMap).Shader_ = m_program;

	std::string  fspath2 = m_resourcePath + "/Shader/3D/irradiance_convolution.fs";
	m_programIRR = std::make_shared<CCProgram>();
	m_programIRR->CreatePorgramForFile(vspath.c_str(), fspath2.c_str());
	GET_SHADER_STRUCT_MEMBER(IBL_Irradiance).Shader_ = m_programIRR;

	std::string  fspath3 = m_resourcePath + "/Shader/3D/prefilter.fs";
	m_programPre = std::make_shared<CCProgram>();
	m_programPre->CreatePorgramForFile(vspath.c_str(), fspath3.c_str());
	GET_SHADER_STRUCT_MEMBER(IBL_Prefilter).Shader_ = m_programPre;

	std::string  vspath4 = m_resourcePath + "/Shader/3D/brdf.vs";
	std::string  fspath4 = m_resourcePath + "/Shader/3D/brdf.fs";
	m_programBlut = std::make_shared<CCProgram>();
	m_programBlut->CreatePorgramForFile(vspath4.c_str(), fspath4.c_str());
	GET_SHADER_STRUCT_MEMBER(IBL_Brdf).Shader_ = m_programBlut;

	Initialize();
}