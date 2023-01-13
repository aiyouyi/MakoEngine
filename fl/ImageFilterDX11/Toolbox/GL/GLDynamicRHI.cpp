#include "GLDynamicRHI.h"
#include "GLResource.h"
#include "BaseDefine/Define.h"
#include "DoubleBuffer.h"
#include "GLShaderRHI.h"

uint32_t GLInternalFormats[] = {
		GL_RGBA,
		0,
		0,
		0,
		GL_RGBA32F,
		GL_RGB32F,
		GL_RGBA16F,
		GL_RGB16F,
		GL_RG16F,
		GL_R16F,
		GL_R32F,
		0,
		0,
		GL_RGB32F,//使用RG 当作 RenderTarget 绑定到 FRAMEBUFFER 的时候会失败，所以OPENGL使用RG也当作使用RGB
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		//Compress Type
		0,
		0,
		0,
		0,
		0
};

std::unordered_map<uint32_t, uint32_t> InternalFormatMap;
std::unordered_map<uint32_t, uint32_t> FormatType;


std::tuple<uint32_t, uint32_t, uint32_t> ConvertGLTextureFormat(uint32_t format)
{
	uint32_t InternalFormat = GLInternalFormats[format];
	auto itFoundFormat = InternalFormatMap.find(InternalFormat);
	if (itFoundFormat == InternalFormatMap.end())
	{
		return { 0,0,0 };
	}

	auto itFoundType = FormatType.find(InternalFormat);
	if (itFoundType == FormatType.end())
	{
		return { 0,0,0 };
	}
	return { InternalFormat ,itFoundType->second,itFoundFormat->second };
}


GLDynamicRHI* g_Instance = nullptr;

GLDynamicRHI::GLDynamicRHI()
{
	if (InternalFormatMap.empty())
	{
		InternalFormatMap.insert({ GL_RGBA ,GL_RGBA });
		//InternalFormatMap.insert({ GL_BGRA ,GL_BGRA });
		InternalFormatMap.insert({ GL_RGBA32F ,GL_RGBA });
		InternalFormatMap.insert({ GL_RGB32F ,GL_RGB });
		InternalFormatMap.insert({ GL_RGB16F ,GL_RGB });
		InternalFormatMap.insert({ GL_RGBA16F ,GL_RGBA });
		InternalFormatMap.insert({ GL_RG32F ,GL_RG32F });
		InternalFormatMap.insert({ GL_RG16F ,GL_RG16F });
		// InternalFormatMap.insert({ GL_RG16 ,GL_RG });
		// InternalFormatMap.insert({ GL_R32F ,GL_R });
		// InternalFormatMap.insert({ GL_R8 ,GL_R });
		// InternalFormatMap.insert({ GL_R16 ,GL_R });
	}

	if (FormatType.empty())
	{
		FormatType.insert({ GL_RGBA ,GL_UNSIGNED_BYTE });
		//FormatType.insert({ GL_BGRA ,GL_UNSIGNED_BYTE });
		// FormatType.insert({ GL_RG16 ,GL_UNSIGNED_BYTE });
		// FormatType.insert({ GL_R8 ,GL_UNSIGNED_BYTE });
		// FormatType.insert({ GL_R16 ,GL_UNSIGNED_BYTE });
		FormatType.insert({ GL_RGBA32F ,GL_FLOAT });
		FormatType.insert({ GL_RGB32F ,GL_FLOAT });
		FormatType.insert({ GL_RGB16F ,GL_FLOAT });
		FormatType.insert({ GL_RGBA16F ,GL_FLOAT });
		FormatType.insert({ GL_RG16F ,GL_FLOAT });
		// FormatType.insert({ GL_R32F ,GL_FLOAT });
		FormatType.insert({ GL_RG32F ,GL_FLOAT });
	}
}

GLDynamicRHI::~GLDynamicRHI()
{

}

GLDynamicRHI* GLDynamicRHI::GetInstance()
{
	if (!g_Instance)
	{
		g_Instance = new GLDynamicRHI();
	}
	return g_Instance;
}

void GLDynamicRHI::Release()
{
	delete g_Instance;
	g_Instance = nullptr;
}

std::shared_ptr<CC3DIndexBuffer> GLDynamicRHI::CreateIndexBuffer(unsigned short* pData, int nTriangle)
{
	std::shared_ptr<GLIndexBuffer> IndexBuffer = std::make_shared<GLIndexBuffer>();
	IndexBuffer->CreateIndexBuffer(pData, nTriangle);
	return IndexBuffer;
}

std::shared_ptr<CC3DIndexBuffer> GLDynamicRHI::CreateIndexBuffer(unsigned int* pData, int nTriangle)
{
	std::shared_ptr<GLIndexBuffer> IndexBuffer = std::make_shared<GLIndexBuffer>();
	IndexBuffer->CreateIndexBuffer(pData, nTriangle);
	return IndexBuffer;
}

std::shared_ptr<CC3DVertexBuffer> GLDynamicRHI::CreateVertexBuffer(int buffercount)
{
	std::shared_ptr<GLVertexBuffer> VertexBuffer = std::make_shared<GLVertexBuffer>();
	VertexBuffer->InitBufferCount(buffercount);
	return VertexBuffer;
}

std::shared_ptr<CC3DVertexBuffer> GLDynamicRHI::CreateVertexBuffer(float* pData, int nVertex, int nStride)
{
	std::shared_ptr<GLVertexBuffer> VertexBuffer = std::make_shared<GLVertexBuffer>();
	VertexBuffer->CreateVertexBuffer(pData, nVertex, nStride);
	return VertexBuffer;
}

std::shared_ptr<CC3DVertexBuffer> GLDynamicRHI::CreateVertexBuffer(float* pVertices, int nVerts, int* pArrange, int nArrange, int nStep)
{
	std::shared_ptr<GLVertexBuffer> VertexBuffer = std::make_shared<GLVertexBuffer>();
	VertexBuffer->CreateVertexBuffer(pVertices, nVerts, pArrange,nArrange,nStep);
	return VertexBuffer;
}

std::shared_ptr< CC3DBlendState > GLDynamicRHI::CreateBlendState(int32_t SrcBlend /*= CC3DBlendState::BP_SRCALPHA*/, int32_t DstBlend /*= CC3DBlendState::BP_INVSRCALPHA*/)
{
	std::shared_ptr<GLBlendState> BlendState = std::make_shared<GLBlendState>();
	BlendState->bAlphaBlendEnable[0] = true;
	BlendState->bBlendEnable[0] = true;
	BlendState->ucSrcBlend[0] = SrcBlend;
	BlendState->ucDestBlend[0] = DstBlend;
	BlendState->CreateState();
	return BlendState;
}

std::shared_ptr<CC3DBlendState> GLDynamicRHI::CreateBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer, bool maskRGB)
{
	std::shared_ptr<GLBlendState> BlendState = std::make_shared<GLBlendState>();
	BlendState->bAlphaBlendEnable[0] = bBlendAlpha;
	BlendState->bBlendEnable[0] = bBlend;
	if (bBlendAlpha)
	{
		BlendState->ucSrcBlend[0] = CC3DBlendState::BP_SRCALPHA;
		BlendState->ucDestBlend[0] = CC3DBlendState::BP_INVSRCALPHA;
	}
	else
	{
		BlendState->ucSrcBlend[0] = CC3DBlendState::BP_ONE;
		BlendState->ucDestBlend[0] = CC3DBlendState::BP_ONE;
	}
	BlendState->CreateState();
	return BlendState;
}

std::shared_ptr<CC3DDepthStencilState> GLDynamicRHI::CreateDefaultStencilState(bool enableDepthTest, bool enableDepthWrite)
{
	std::shared_ptr<CC3DDepthStencilState> DepthStencilState = std::make_shared<GLDepthStencilState>();
	DepthStencilState->m_bDepthEnable = enableDepthTest;
	DepthStencilState->m_bDepthWritable = enableDepthWrite;
	return DepthStencilState;
}

std::shared_ptr<CC3DDepthStencilState> GLDynamicRHI::CreateDepthStencilState()
{
	std::shared_ptr<CC3DDepthStencilState> DepthStencilState = std::make_shared<GLDepthStencilState>();
	return DepthStencilState;
}

std::shared_ptr<CC3DSamplerState> GLDynamicRHI::CreateSampler(int32_t Mode /*= CC3DSamplerState::CM_CLAMP*/, int32_t FilterMin /*= CC3DSamplerState::FM_LINE*/, int32_t FilterMag /*= CC3DSamplerState::FM_LINE*/, int32_t FilterMip /*= CC3DSamplerState::FM_LINE*/)
{
	std::shared_ptr<CC3DSamplerState> SamplerStateRHI = std::make_shared<GLSamplerState>();
	SamplerStateRHI->m_uiCoordU = Mode;
	SamplerStateRHI->m_uiCoordV = Mode;
	SamplerStateRHI->m_uiCoordW = Mode;
	SamplerStateRHI->m_uiMin = FilterMin;
	SamplerStateRHI->m_uiMag = FilterMag;
	SamplerStateRHI->m_uiMip = FilterMip;
	SamplerStateRHI->CreateState();
	return SamplerStateRHI;
}

std::shared_ptr<CC3DRasterizerState> GLDynamicRHI::CreateRasterizerState(int32_t CullType /*= CC3DRasterizerState::CT_NONE*/, bool bWireEnable /*= false*/)
{
	std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI = std::make_shared<GLRasterizerState>();
	RasterizerStateRHI->m_uiCullType = CullType;
	RasterizerStateRHI->m_bWireEnable = bWireEnable;
	RasterizerStateRHI->CreateState();
	return RasterizerStateRHI;
}

std::shared_ptr<CC3DConstantBuffer> GLDynamicRHI::CreateConstantBuffer(int32_t ByteSize)
{
	std::shared_ptr<CC3DConstantBuffer> ConstantBuffer = std::make_shared<GLConstantBuffer>();
	ConstantBuffer->CreateBuffer(ByteSize);
	return ConstantBuffer;
}

std::shared_ptr< CC3DTextureRHI> GLDynamicRHI::CreateTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer /*= nullptr*/, int rowBytes /*= 0*/, bool bGenMipmap /*= false*/, bool bMultSample /*= false*/)
{
	std::shared_ptr<CC3DTextureRHI> Texture2D = std::make_shared<GLTexture2D>();
	if (!Texture2D->InitTexture(format, BindFlags, width, height, pBuffer, rowBytes, bGenMipmap, bMultSample))
	{
		return nullptr;
	}
	return Texture2D;
}

std::shared_ptr<CC3DTextureRHI> GLDynamicRHI::CreateTexture()
{
	std::shared_ptr<CC3DTextureRHI> Texture2D = std::make_shared<GLTexture2D>();
	return Texture2D;
}

std::shared_ptr<CC3DTextureRHI> GLDynamicRHI::CreateTextureFromFile(const std::string szFile, bool bGenMipmap /*= false*/)
{
	std::shared_ptr<CC3DTextureRHI> Texture2D = std::make_shared<GLTexture2D>();
	if (!Texture2D->InitTextureFromFile(szFile,bGenMipmap))
	{
		return nullptr;
	}
	return Texture2D;
}

std::shared_ptr<CC3DTextureRHI> GLDynamicRHI::CreateTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap /*= false*/)
{
	std::shared_ptr<CC3DTextureRHI> Texture2D = std::make_shared<GLTexture2D>();
	if (!Texture2D->InitTextureFromZip(hZip, szImagePath,bGenMipmap))
	{
		return nullptr;
	}
	return Texture2D;
}

std::shared_ptr<CC3DTextureRHI> GLDynamicRHI::CreateTextureFromFileCPUAcess(const std::string szFile)
{
	std::shared_ptr<CC3DTextureRHI> Texture2D = std::make_shared<GLTexture2D>();
	if (!Texture2D->InitTextureFromFileCPUAcess(szFile))
	{
		return nullptr;
	}
	return Texture2D;
}

std::shared_ptr<CC3DRenderTargetRHI> GLDynamicRHI::CreateRenderTarget(int width, int height, bool useDepthBuffer /*= false*/, std::shared_ptr< CC3DTextureRHI> pDestTexture /*= nullptr*/, uint8_t format /*= 0*/)
{
	std::shared_ptr<GLRenderTarget> RenderTarget = std::make_shared<GLRenderTarget>();
	if (!RenderTarget->InitWithTexture(width,height, useDepthBuffer,pDestTexture,format))
	{
		return nullptr;
	}
	return RenderTarget;
}

std::shared_ptr<CC3DCubeMapRHI> GLDynamicRHI::CreateCubeMap(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget)
{
	std::shared_ptr<GLCubeMap> CubeMap = std::make_shared<GLCubeMap>();
	if (!CubeMap->Init(nTextureWidth, nTexureHeight, nMipLevels, UseDepth, GenMipRetarget))
	{
		return nullptr;
	}
	return CubeMap;
}

void GLDynamicRHI::SetBlendState(std::shared_ptr< CC3DBlendState > BlendStateRHI, const float BlendFactor[4], uint32_t SimpleMask)
{
	GLBlendState* BlendState = RHIResourceCast(BlendStateRHI.get());
	if (BlendState->IsBlendEnable())
	{
		glEnable(GL_BLEND);
		glColorMask(1, 1, 1, 1);
		glBlendFunc(BlendState->Sfactor(), BlendState->Dfactor());
	}
	else
	{
		glDisable(GL_BLEND);
		glColorMask(1, 1, 1, 0);
	}
}

void GLDynamicRHI::SetDepthStencilState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI, uint32_t StencilRef /*= 0*/)
{
	//UNREFERENCED_PARAMETER(StencilRef);
	GLDepthStencilState* DepthStencilState = RHIResourceCast(DepthStencilStateRHI.get());
	if (DepthStencilState->IsDepthEnable())
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(DepthStencilState->DepthCompare());
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}


}

void GLDynamicRHI::SetDepthWritableState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI)
{
	GLDepthStencilState* DepthStencilState = RHIResourceCast(DepthStencilStateRHI.get());
	if (DepthStencilState->m_bDepthWritable)
	{
		glDepthMask(GL_TRUE);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}
}

void GLDynamicRHI::SetSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI, int slot)
{
	//UNREFERENCED_PARAMETER(SamplerStateRHI);
}

void GLDynamicRHI::SetRasterizerState(std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI)
{
	GLRasterizerState* RasterizerState = RHIResourceCast(RasterizerStateRHI.get());
	if (RasterizerState->IsEnableCullFace())
	{
		glEnable(GL_CULL_FACE);
		if (bRasterizeCullInverse)
		{
			unsigned char cullType = 0x03 &(  ~RasterizerState->m_uiCullType);
			glCullFace(RasterizerState->CullMode(cullType));
		}
		else
		{
			glCullFace(RasterizerState->CullMode());
		}
		
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

void GLDynamicRHI::UpdateConstantBuffer(std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI, const void* pSrcData)
{
	//UNREFERENCED_PARAMETER(ConstantBufferRHI);
	//UNREFERENCED_PARAMETER(pSrcData);
}

void GLDynamicRHI::SetVSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI)
{
	//UNREFERENCED_PARAMETER(StartSlot);
	//UNREFERENCED_PARAMETER(ConstantBufferRHI);
}

void GLDynamicRHI::SetPSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI)
{
	//UNREFERENCED_PARAMETER(StartSlot);
	//UNREFERENCED_PARAMETER(ConstantBufferRHI);
}

void GLDynamicRHI::SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{
	GLRenderTarget* RenderTarget = RHIResourceCast(RenderTargetRHI.get());

	glActiveTexture(GL_TEXTURE0+StartSlot);
	glBindTexture(GL_TEXTURE_2D, RenderTarget->GetTextureId());
}

void GLDynamicRHI::SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	GLTexture2D* Texture2D = RHIResourceCast(TextureRHI.get());

	glActiveTexture(GL_TEXTURE0 + StartSlot);
	glBindTexture(GL_TEXTURE_2D, Texture2D->GetTextureID());
}

void GLDynamicRHI::SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DCubeMapRHI> TextureRHI)
{
	//UNREFERENCED_PARAMETER(StartSlot);
	GLCubeMap* CubeMap = RHIResourceCast(TextureRHI.get());
	glActiveTexture(GL_TEXTURE0 + StartSlot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap->GetTextureId());
}

bool GLDynamicRHI::UpdateTextureInfo(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer, int32_t w, int32_t h)
{
	GLTexture2D* Texture2D = RHIResourceCast(TextureRHI.get());
	//UNREFERENCED_PARAMETER(pBuffer);
	//UNREFERENCED_PARAMETER(w);
	//UNREFERENCED_PARAMETER(h);
	return false;
}

bool GLDynamicRHI::ReadTextureToCpu(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer)
{
	GLTexture2D* Texture2D = RHIResourceCast(TextureRHI.get());
	//UNREFERENCED_PARAMETER(pBuffer);
	return false;
}

void GLDynamicRHI::DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI)
{
	GLVertexBuffer* VertexBuffer = RHIResourceCast(VertexBufferRHI.get());
	GLIndexBuffer* IndexBuffer = RHIResourceCast(IndexBufferRHI.get());

	glBindVertexArray(VertexBuffer->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer->EBO);

	glDrawElements(GL_TRIANGLES, IndexBuffer->GetNumberTriangle()* 3, IndexBuffer->GetIndexFormat(), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void GLDynamicRHI::DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI)
{
	GLVertexBuffer* VertexBuffer = RHIResourceCast(VertexBufferRHI.get());

	glBindVertexArray(VertexBuffer->VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, VertexBuffer->GetCount());

	glBindVertexArray(0);
}

void GLDynamicRHI::DrawPrimitiveTriangles(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI)
{
	GLVertexBuffer* VertexBuffer = RHIResourceCast(VertexBufferRHI.get());

	glBindVertexArray(VertexBuffer->VAO);
	glDrawArrays(GL_TRIANGLES, 0, VertexBuffer->GetCount());

	glBindVertexArray(0);
}


void GLDynamicRHI::DrawPrimitive(const std::array<std::shared_ptr<CC3DVertexBuffer>, 10>& VertexBufferRHI, int32_t VertexBufferCount, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI)
{
	if (VertexBufferRHI[0] == nullptr)
	{
		return;
	}
	GLVertexBuffer* VertexBuffer = RHIResourceCast(VertexBufferRHI[0].get());
	GLIndexBuffer* IndexBuffer = RHIResourceCast(IndexBufferRHI.get());

	glBindVertexArray(VertexBuffer->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer->EBO);

	glDrawElements(GL_TRIANGLES, IndexBuffer->GetNumberTriangle() * 3, IndexBuffer->GetIndexFormat(), 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void GLDynamicRHI::GenerateMips(std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI)
{
	GLCubeMap* CubeMap = RHIResourceCast(CubeMapRHI.get());
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap->GetTextureId());
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GLDynamicRHI::SetViewPort(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth /*= 0.0*/, float MaxDepth /*= 1.0*/)
{
	//UNREFERENCED_PARAMETER(MinDepth);
	//UNREFERENCED_PARAMETER(MaxDepth);
	glViewport(TopLeftX, TopLeftY, Width, Height);
}

void GLDynamicRHI::SetCullInverse(bool value)
{
	bRasterizeCullInverse = value;
}

void GLDynamicRHI::SetColorMask(float r, float g, float b, float a)
{
	glColorMask(r, g, b, a);
}

std::shared_ptr< DoubleBufferRHI> GLDynamicRHI::CreateDoubleBuffer()
{
	return std::make_shared< GLDoubleBuffer>();
}

std::shared_ptr< ShaderRHI> GLDynamicRHI::CreateShaderRHI()
{
	return std::make_shared< GLShaderRHI>();
}
