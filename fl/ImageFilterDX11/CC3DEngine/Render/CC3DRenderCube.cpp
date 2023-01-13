#include "CC3DRenderCube.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CC3DRenderCube::CC3DRenderCube()
{
	m_program = NULL;
	m_programBlut = NULL;
	m_programPre = NULL;
	m_programIRR = NULL;
}


CC3DRenderCube::~CC3DRenderCube()
{
	SAFE_DELETE(m_program);
	SAFE_DELETE(m_programBlut);
	SAFE_DELETE(m_programPre);
	SAFE_DELETE(m_programIRR);
	
}

void CC3DRenderCube::Initialize()
{
	m_envCube = GetDynamicRHI()->CreateCubeMap(512, 512, 5, false, false);
	m_IrrCube = GetDynamicRHI()->CreateCubeMap(32, 32,5, false, false);
	m_PreCube = GetDynamicRHI()->CreateCubeMap(128, 128,8,false,true);

	m_BlutTexture = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_A16B16G16R16, CC3DTextureRHI::OT_RENDER_TARGET, 512, 512,nullptr);
	m_Blut = GetDynamicRHI()->CreateRenderTarget(512, 512, false, m_BlutTexture);
	m_IBLCB = GetDynamicRHI()->CreateConstantBuffer(sizeof(IBLConstantBuffer));

	Vector3 arrCoords[4] = { Vector3(-1, -1, 1), Vector3(1, -1, 1), Vector3(-1, 1, 1), Vector3(1, 1, 1) };

	unsigned short index[] =
	{
		0, 1, 2,
		1, 2, 3
	};
	m_QuadVertexBuffer = GetDynamicRHI()->CreateVertexBuffer((float*)arrCoords, _countof(arrCoords), 3);
	m_QuadIndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);
}


void CC3DRenderCube::Render()
{
	if (m_hdrCube == 0)
	{
		return;
	}
	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
 	glm::mat4 captureProjection = glm::transpose(glm::perspective(glm::radians(90.f), 1.0f, 0.1f, 10.0f));
// 	glm::mat4 captureViews[] =
// 	{
// 		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))),
// 		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))),
// 		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))),
// 		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))),
// 		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))),
// 		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)))
// 	};

	glm::mat4 captureViews[] =
	{
		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))),
		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f))),
		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))),
		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))),
		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f))),
		glm::transpose(glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
	};

	m_program->useShader();
	m_IBLConstantBuffer.projection = captureProjection;

	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	GetDynamicRHI()->SetPSShaderResource(0, m_hdrCube);

	for (int i=0;i<6;i++)
	{
		m_IBLConstantBuffer.view = captureViews[i];
		m_envCube->SetRenderTarget(i);
		GetDynamicRHI()->UpdateConstantBuffer(m_IBLCB, &m_IBLConstantBuffer);
		GetDynamicRHI()->SetVSConstantBuffer(0, m_IBLCB);
		GetDynamicRHI()->SetPSConstantBuffer(0, m_IBLCB);

		renderCube();
	}

	GetDynamicRHI()->GenerateMips(m_envCube);

	m_programIRR->useShader();

	for (int i = 0; i < 6; i++)
	{
		m_IBLConstantBuffer.view = captureViews[i];
		m_IrrCube->SetRenderTarget(i);
		GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

		GetDynamicRHI()->SetPSShaderResource(0, m_envCube);
		GetDynamicRHI()->UpdateConstantBuffer(m_IBLCB, &m_IBLConstantBuffer);
		GetDynamicRHI()->SetVSConstantBuffer(0, m_IBLCB);
		GetDynamicRHI()->SetPSConstantBuffer(0, m_IBLCB);
		renderCube();
	}
	GetDynamicRHI()->GenerateMips(m_IrrCube);
	m_programPre->useShader();

	unsigned int maxMipLevels = 8;

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_IBLConstantBuffer.Roughness.x = roughness;

		
		for (unsigned int i = 0; i < 6; ++i)
		{
			m_PreCube->SetRenderTarget(i, mip);
			GetDynamicRHI()->SetViewPort(0, 0, mipWidth, mipHeight);
			GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);

			m_IBLConstantBuffer.view = captureViews[i];
			GetDynamicRHI()->SetPSShaderResource(0, m_envCube);
			GetDynamicRHI()->UpdateConstantBuffer(m_IBLCB, &m_IBLConstantBuffer);
			GetDynamicRHI()->SetVSConstantBuffer(0, m_IBLCB);
			GetDynamicRHI()->SetPSConstantBuffer(0, m_IBLCB);
		
			renderCube();
		}

	}

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullNone);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BlendDisable, blendFactor, 0xffffffff);

	m_programBlut->useShader();
	m_Blut->Bind();
	
	GetDynamicRHI()->DrawPrimitive(m_QuadVertexBuffer, m_QuadIndexBuffer);

}


void CC3DRenderCube::renderCube()
{
	// initialize (if necessary)
	if (m_cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,    
			1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f, 
			-1.0f, -1.0f, 1.0f, 
			1.0f, -1.0f, 1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f, 1.0f, 1.0f, 
			-1.0f, 1.0f, 1.0f, 
			-1.0f, -1.0f, 1.0f, 
			-1.0f, 1.0f, 1.0f, 
			-1.0f, 1.0f, -1.0f, 
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f, 
			-1.0f, 1.0f, 1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f, -1.0f, -1.0f, 
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f, -1.0f, 1.0f, 
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f, 
			1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f, 
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f, 1.0f, -1.0f, 
			1.0f, 1.0f, 1.0f, 
			-1.0f, 1.0f, -1.0f, 
			-1.0f, 1.0f, 1.0f, 
		};
		m_cubeVAO = GetDynamicRHI()->CreateVertexBuffer(vertices, 36, 3);
	}
	// render Cube
	GetDynamicRHI()->DrawPrimitive(m_cubeVAO);
}

void CC3DRenderCube::setHDR(const std::string& filePath)
{
	HDRFilePath = filePath;
	int width, height, nrComponents;
	float *data = ccLoadImagef(HDRFilePath.c_str(), &width, &height, &nrComponents,0);
	if (data)
	{
		m_hdrCube = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R32G32B32F, 0, width, height, data, width * 3 * sizeof(float));

		SAFE_DELETE_ARRAY(data);
	}
	else
	{
		LOGE("Failed to load HDR image.");
	}
}


void CC3DRenderCube::setHDRData(float *pData, int nWidth, int nHeight)
{
	if (pData)
	{
		m_hdrCube = GetDynamicRHI()->CreateTexture(CC3DTextureRHI::SFT_R32G32B32F, 0, nWidth, nHeight, pData, nWidth * 3 * sizeof(float));
	}
	else
	{
		LOGE("Failed to load HDR image.");
	}
}


void CC3DRenderCube::SetShaderResource(const std::string& path)
{
	m_resourcePath = path;
	if (m_program != NULL)
	{
		return;
	}

	m_program = new DX11Shader();
	m_programIRR = new DX11Shader();
	m_programPre = new DX11Shader();
	m_programBlut = new DX11Shader();
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
	};
	std::string  fspath = m_resourcePath + "/Shader/3D/equirectangular_to_cubemap.fx";
	DXUtils::CompileShaderWithFile(m_program, (char*)fspath.c_str(), pAttribute, 1);

	fspath = m_resourcePath + "/Shader/3D/irradiance_convolution.fx";
	DXUtils::CompileShaderWithFile(m_programIRR, (char*)fspath.c_str(), pAttribute, 1);

	fspath = m_resourcePath + "/Shader/3D/prefilter.fx";
	DXUtils::CompileShaderWithFile(m_programPre, (char*)fspath.c_str(), pAttribute, 1);


	fspath = m_resourcePath + "/Shader/3D/brdf.fx";
	DXUtils::CompileShaderWithFile(m_programBlut, (char*)fspath.c_str(), pAttribute, 1);

	Initialize();
}