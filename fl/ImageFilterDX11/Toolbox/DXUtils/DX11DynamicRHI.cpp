#include "DX11DynamicRHI.h"
#include "DX11Resource.h"
#include "DX11Context.h"
#include "Toolbox/DXUtils/DXUtils.h"

CC3DDynamicRHI* GetDynamicRHI()
{
	return DX11DynamicRHI::GetInstance();
}

DX11DynamicRHI* g_Instance = nullptr;

DX11DynamicRHI::DX11DynamicRHI()
{

}

DX11DynamicRHI::~DX11DynamicRHI()
{

}

DX11DynamicRHI* DX11DynamicRHI::GetInstance()
{
	if (!g_Instance)
	{
		g_Instance = new DX11DynamicRHI();
	}
	return g_Instance;
}

void DX11DynamicRHI::Release()
{
	delete g_Instance;
	g_Instance = nullptr;
}

std::shared_ptr<CC3DIndexBuffer> DX11DynamicRHI::CreateIndexBuffer(unsigned short* pData, int nTriangle)
{
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer = std::make_shared<DX11IndexBuffer>();
	IndexBuffer->CreateIndexBuffer(pData, nTriangle);
	return IndexBuffer;
}

std::shared_ptr<CC3DIndexBuffer> DX11DynamicRHI::CreateIndexBuffer(unsigned int* pData, int nTriangle)
{
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer = std::make_shared<DX11IndexBuffer>();
	IndexBuffer->CreateIndexBuffer(pData, nTriangle);
	return IndexBuffer;
}

std::shared_ptr<CC3DVertexBuffer> DX11DynamicRHI::CreateVertexBuffer(float* pData, int nVertex, int nStride)
{
	std::shared_ptr<CC3DVertexBuffer> VertexBuffer = std::make_shared<DX11VertexBuffer>();
	VertexBuffer->CreateVertexBuffer(pData, nVertex, nStride);
	return  VertexBuffer;
}

std::shared_ptr< CC3DBlendState > DX11DynamicRHI::CreateBlendState(int32_t SrcBlend /*= CC3DBlendState::BP_SRCALPHA*/, int32_t DstBlend /*= CC3DBlendState::BP_INVSRCALPHA*/)
{
	std::shared_ptr< CC3DBlendState > BS = std::make_shared<DX11BlendState>();

	BS->bAlphaToCoverageEnable = false;
	BS->bIndependentBlendEnable = false;
	BS->bAlphaBlendEnable[0] = true;
	BS->bBlendEnable[0] = true;
	BS->ucBlendOp[0] = CC3DBlendState::BO_ADD;
	BS->ucBlendOpAlpha[0] = CC3DBlendState::BO_ADD;

	BS->ucSrcBlendAlpha[0] = CC3DBlendState::BP_ONE;
	BS->ucDestBlendAlpha[0] = CC3DBlendState::BP_ONE;

	BS->ucSrcBlend[0] = SrcBlend;
	BS->ucDestBlend[0] = DstBlend;

	BS->ucWriteMask[0] = D3D11_COLOR_WRITE_ENABLE_ALL;

	BS->CreateState();
	return BS;
}

std::shared_ptr<CC3DBlendState> DX11DynamicRHI::CreateBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer)
{
	std::shared_ptr< DX11BlendState > BS = std::make_shared<DX11BlendState>();
	BS->CreateState(bBlend, bBlendAlpha, writeBuffer);
	return BS;
}

std::shared_ptr<CC3DDepthStencilState> DX11DynamicRHI::CreateDefaultStencilState(bool enableDepthTest, bool enableDepthWrite)
{
	std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI = std::make_shared<DX11DepthStencilState>();

	DepthStencilStateRHI->m_bDepthEnable = enableDepthTest;
	DepthStencilStateRHI->m_uiDepthCompareMethod = CC3DDepthStencilState::CM_LESSEQUAL;
	DepthStencilStateRHI->m_bDepthWritable = enableDepthWrite;
	DepthStencilStateRHI->m_bStencilEnable = false;

	DepthStencilStateRHI->CreateState();
	return DepthStencilStateRHI;
}

std::shared_ptr<CC3DDepthStencilState> DX11DynamicRHI::CreateDepthStencilState()
{
	return make_shared<DX11DepthStencilState>();
}

std::shared_ptr<CC3DSamplerState> DX11DynamicRHI::CreateSampler(int32_t Mode /*= CC3DSamplerState::CM_CLAMP*/, int32_t FilterMin /*= CC3DSamplerState::FM_LINE*/, int32_t FilterMag /*= CC3DSamplerState::FM_LINE*/, int32_t FilterMip /*= CC3DSamplerState::FM_LINE*/)
{
	std::shared_ptr<CC3DSamplerState> SamplerStateRHI = std::make_shared<DX11SampleState>();
	SamplerStateRHI->m_uiCoordU = Mode;
	SamplerStateRHI->m_uiCoordV = Mode;
	SamplerStateRHI->m_uiCoordW = Mode;
	SamplerStateRHI->m_uiMin = FilterMin;
	SamplerStateRHI->m_uiMag = FilterMag;
	SamplerStateRHI->m_uiMip = FilterMip;
	SamplerStateRHI->CreateState();
	return SamplerStateRHI;
}

std::shared_ptr<CC3DRasterizerState> DX11DynamicRHI::CreateRasterizerState(int32_t CullType /*= CC3DRasterizerState::CT_NONE*/, bool bWireEnable /*= false*/)
{
	std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI = std::make_shared<DX11RasterizerState>();
	RasterizerStateRHI->m_uiCullType = CullType;
	RasterizerStateRHI->m_bWireEnable = bWireEnable;
	RasterizerStateRHI->CreateState();
	return RasterizerStateRHI;
}

std::shared_ptr<CC3DConstantBuffer> DX11DynamicRHI::CreateConstantBuffer(int32_t ByteSize)
{
	std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI = std::make_shared<DX11ConstantBuffer>();
	ConstantBufferRHI->CreateBuffer(ByteSize);
	return ConstantBufferRHI;
}

std::shared_ptr< CC3DTextureRHI> DX11DynamicRHI::CreateTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer /*= nullptr*/, int rowBytes /*= 0*/, bool bGenMipmap /*= false*/, bool bMultSample /*= false*/)
{
	std::shared_ptr<CC3DTextureRHI> TextureRHI = std::make_shared<DX11Texture2D>();
	if (TextureRHI->InitTexture(format, BindFlags, width, height, pBuffer, rowBytes, bGenMipmap, bMultSample))
	{
		return TextureRHI;
	}
	return nullptr;
}

std::shared_ptr<CC3DTextureRHI> DX11DynamicRHI::CreateTextureFromFile(const std::string szFile, bool bGenMipmap /*= false*/)
{
	std::shared_ptr<CC3DTextureRHI> TextureRHI = std::make_shared<DX11Texture2D>();
	if (TextureRHI->InitTextureFromFile(szFile, bGenMipmap))
	{
		return TextureRHI;
	}
	return nullptr;
}

std::shared_ptr<CC3DTextureRHI> DX11DynamicRHI::CreateTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap /*= false*/)
{
	std::shared_ptr<CC3DTextureRHI> TextureRHI = std::make_shared<DX11Texture2D>();
	if (TextureRHI->InitTextureFromZip(hZip, szImagePath, bGenMipmap))
	{
		return TextureRHI;
	}
	return nullptr;
}

std::shared_ptr<CC3DTextureRHI> DX11DynamicRHI::CreateTextureFromFileCPUAcess(const std::string szFile)
{
	std::shared_ptr<CC3DTextureRHI> TextureRHI = std::make_shared<DX11Texture2D>();
	if (TextureRHI->InitTextureFromFileCPUAcess(szFile))
	{
		return TextureRHI;
	}
	return nullptr;
}

std::shared_ptr<CC3DRenderTargetRHI> DX11DynamicRHI::CreateRenderTarget(int width, int height, bool useDepthBuffer /*= false*/, std::shared_ptr< CC3DTextureRHI> pDestTexture /*= nullptr*/, uint8_t format /*= 0*/)
{
	std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI = std::make_shared<DX11RenderTarget>();
	if (RenderTargetRHI->InitWithTexture(width, height, useDepthBuffer, pDestTexture, format))
	{
		return RenderTargetRHI;
	}
	return nullptr;
}

std::shared_ptr<CC3DCubeMapRHI> DX11DynamicRHI::CreateCubeMap(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget)
{
	std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI = std::make_shared<DX11CubeMapRHI>();
	if (CubeMapRHI->Init(nTextureWidth,nTexureHeight,nMipLevels,UseDepth,GenMipRetarget))
	{
		return CubeMapRHI;
	}
	return nullptr;
}

void DX11DynamicRHI::SetBlendState(std::shared_ptr< CC3DBlendState > BlendStateRHI, const float BlendFactor[4], uint32_t SimpleMask)
{
	DX11BlendState* BlendState = RHIResourceCast(BlendStateRHI.get());
	if (BlendState && BlendState->PtrBlendState)
	{
		DeviceContextPtr->OMSetBlendState(BlendState->PtrBlendState, BlendFactor, SimpleMask);
	}
}

void DX11DynamicRHI::SetDepthStencilState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI, uint32_t StencilRef )
{
	DX11DepthStencilState* DepthStencilState = RHIResourceCast(DepthStencilStateRHI.get());
	if (DepthStencilState && DepthStencilState->PtrDepthState)
	{
		DeviceContextPtr->OMSetDepthStencilState(DepthStencilState->PtrDepthState, StencilRef);
	}
	
}

void DX11DynamicRHI::SetSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI)
{
	DX11SampleState* SampleState = RHIResourceCast(SamplerStateRHI.get());
	if (SampleState && SampleState->PtrSamplerState)
	{
		DeviceContextPtr->PSSetSamplers(0, 1, &SampleState->PtrSamplerState);
	}
	
}

void DX11DynamicRHI::SetVSSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI)
{
	DX11SampleState* SampleState = RHIResourceCast(SamplerStateRHI.get());
	if (SampleState && SampleState->PtrSamplerState)
	{
		DeviceContextPtr->VSSetSamplers(0, 1, &SampleState->PtrSamplerState);
	}
}

void DX11DynamicRHI::SetRasterizerState(std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI)
{
	DX11RasterizerState* RasterizerState = RHIResourceCast(RasterizerStateRHI.get());
	if (RasterizerState && RasterizerState->PtrRasterizerState)
	{
		DeviceContextPtr->RSSetState(RasterizerState->PtrRasterizerState);
	}
	
}

void DX11DynamicRHI::UpdateConstantBuffer(std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI,const void* pSrcData)
{
	DX11ConstantBuffer* ConstantBuffer = RHIResourceCast(ConstantBufferRHI.get());
	if (ConstantBuffer && ConstantBuffer->PtrConstantBuffer)
	{
		DeviceContextPtr->UpdateSubresource(ConstantBuffer->PtrConstantBuffer, 0, nullptr, pSrcData, 0, 0);
	}
}

void DX11DynamicRHI::SetVSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI)
{
	DX11ConstantBuffer* ConstantBuffer = RHIResourceCast(ConstantBufferRHI.get());
	if (ConstantBuffer && ConstantBuffer->PtrConstantBuffer)
	{
		DeviceContextPtr->VSSetConstantBuffers(StartSlot, 1, &ConstantBuffer->PtrConstantBuffer);
	}
}

void DX11DynamicRHI::SetPSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI)
{
	DX11ConstantBuffer* ConstantBuffer = RHIResourceCast(ConstantBufferRHI.get());
	if (ConstantBuffer && ConstantBuffer->PtrConstantBuffer)
	{
		DeviceContextPtr->PSSetConstantBuffers(StartSlot, 1, &ConstantBuffer->PtrConstantBuffer);
	}
}

void DX11DynamicRHI::SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI)
{
	DX11RenderTarget* RenderTarget = RHIResourceCast(RenderTargetRHI.get());
	if (RenderTarget && RenderTarget->GetSRV())
	{
		ID3D11ShaderResourceView* SRV = RenderTarget->GetSRV();
		DeviceContextPtr->PSSetShaderResources(StartSlot, 1, &SRV);
	}
}

void DX11DynamicRHI::SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	DX11Texture2D* Texture2D = RHIResourceCast(TextureRHI.get());
	if (Texture2D && Texture2D->GetSRV())
	{
		ID3D11ShaderResourceView* SRV = Texture2D->GetSRV();
		DeviceContextPtr->PSSetShaderResources(StartSlot, 1, &SRV);
	}
}

void DX11DynamicRHI::SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DCubeMapRHI> CubeRHI)
{
	DX11CubeMapRHI* CubeMap = RHIResourceCast(CubeRHI.get());
	if (CubeMap && CubeMap->GetSRV())
	{
		ID3D11ShaderResourceView* SRV = CubeMap->GetSRV();
		DeviceContextPtr->PSSetShaderResources(StartSlot, 1, &SRV);
	}
}

void DX11DynamicRHI::SetVSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI)
{
	DX11Texture2D* Texture2D = RHIResourceCast(TextureRHI.get());
	if (Texture2D && Texture2D->GetSRV())
	{
		ID3D11ShaderResourceView* SRV = Texture2D->GetSRV();
		DeviceContextPtr->VSSetShaderResources(StartSlot, 1, &SRV);
	}
}

bool DX11DynamicRHI::UpdateTextureInfo(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer, int32_t w, int32_t h)
{
	DX11Texture2D* Texture2D = RHIResourceCast(TextureRHI.get());
	if (Texture2D && Texture2D->GetNativeTex())
	{
		DeviceContextPtr->UpdateSubresource(Texture2D->GetNativeTex(), 0, NULL, pBuffer, w, 0);
		return true;
	}
	return false;
}

bool DX11DynamicRHI::ReadTextureToCpu(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer)
{
	DX11Texture2D* Texture2D = RHIResourceCast(TextureRHI.get());
	if (Texture2D && Texture2D->GetNativeTex())
	{
		return DXUtils::ReadTextureToCpu(Texture2D->GetNativeTex(), pBuffer, Texture2D->GetWidth(), Texture2D->GetHeight());
	}
	return false;
}

void DX11DynamicRHI::DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI)
{
	DX11VertexBuffer* VertexBuffer = RHIResourceCast(VertexBufferRHI.get());
	DX11IndexBuffer*  IndexBuffer = RHIResourceCast(IndexBufferRHI.get());

	if (!VertexBuffer->PtrVertexBuffer || !IndexBuffer->PtrIndexBuffer)
	{
		return;
	}

	uint32_t offest = 0;
	uint32_t stride = VertexBuffer->GetStride();
	DeviceContextPtr->IASetVertexBuffers(0, 1, &VertexBuffer->PtrVertexBuffer, &stride, &offest);
	DeviceContextPtr->IASetIndexBuffer(IndexBuffer->PtrIndexBuffer, (DXGI_FORMAT)IndexBuffer->GetIndexFormat(), 0);
	DeviceContextPtr->DrawIndexed(IndexBuffer->GetNumberTriangle() * 3, 0, 0);
}

void DX11DynamicRHI::DrawPrimitive(const std::array<std::shared_ptr<CC3DVertexBuffer>, 10>& VertexBufferRHI, int32_t VertexBufferCount, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI)
{
	DX11IndexBuffer* IndexBuffer = RHIResourceCast(IndexBufferRHI.get());
	if (!IndexBuffer->PtrIndexBuffer)
	{
		return;
	}
	ID3D11Buffer* VertexBuffers[10] = {};
	uint32_t Strides[10] = {};
	uint32_t Offset[10] = {};
	for (size_t index = 0; index < VertexBufferCount; ++index)
	{
		if (VertexBufferRHI[index])
		{
			VertexBuffers[index] = RHIResourceCast(VertexBufferRHI[index].get())->PtrVertexBuffer;
			if (!VertexBuffers[index])
			{
				return;
			}
			Strides[index] = VertexBufferRHI[index]->GetStride();
			Offset[index] = 0;
		}
		else
		{
			VertexBuffers[index] = nullptr;
			Strides[index] = 0;
			Strides[index] = 0;
		}

	}

	DeviceContextPtr->IASetVertexBuffers(0, VertexBufferCount, VertexBuffers, Strides, Offset);
	DeviceContextPtr->IASetIndexBuffer(IndexBuffer->PtrIndexBuffer, (DXGI_FORMAT)IndexBuffer->GetIndexFormat(), 0);
	DeviceContextPtr->DrawIndexed(IndexBuffer->GetNumberTriangle() * 3, 0, 0);
}

void DX11DynamicRHI::DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI)
{
	DX11VertexBuffer* VertexBuffer = RHIResourceCast(VertexBufferRHI.get());

	if (!VertexBuffer->PtrVertexBuffer )
	{
		return;
	}

	uint32_t offest = 0;
	uint32_t stride = VertexBuffer->GetStride();
	DeviceContextPtr->IASetVertexBuffers(0, 1, &VertexBuffer->PtrVertexBuffer, &stride, &offest);
	DeviceContextPtr->Draw(VertexBuffer->GetCount(), 0);
}

void DX11DynamicRHI::GenerateMips(std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI)
{
	DeviceContextPtr->GenerateMips(RHIResourceCast(CubeMapRHI.get())->GetSRV());
}

void DX11DynamicRHI::SetViewPort(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth, float MaxDepth)
{
	D3D11_VIEWPORT envMapviewport;
	envMapviewport.Width = Width;
	envMapviewport.Height = Height;
	envMapviewport.MinDepth = MinDepth;
	envMapviewport.MaxDepth = MaxDepth;
	envMapviewport.TopLeftX = TopLeftX;
	envMapviewport.TopLeftY = TopLeftY;
	DeviceContextPtr->RSSetViewports(1, &envMapviewport);
}
