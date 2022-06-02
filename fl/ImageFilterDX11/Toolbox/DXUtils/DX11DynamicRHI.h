#pragma once
#include "Toolbox/Render/DynamicRHI.h"

class DX11DynamicRHI : public CC3DDynamicRHI
{
public:
	DX11DynamicRHI();
	~DX11DynamicRHI();

	static DX11DynamicRHI* GetInstance();
	static void Release();

	virtual std::shared_ptr<CC3DIndexBuffer> CreateIndexBuffer(unsigned short* pData, int nTriangle);
	virtual std::shared_ptr<CC3DIndexBuffer>  CreateIndexBuffer(unsigned int* pData, int nTriangle);
	virtual std::shared_ptr<CC3DVertexBuffer> CreateVertexBuffer(float* pData, int nVertex, int nStride);
	virtual std::shared_ptr< CC3DBlendState > CreateBlendState(int32_t SrcBlend = CC3DBlendState::BP_SRCALPHA, int32_t DstBlend = CC3DBlendState::BP_INVSRCALPHA);
	virtual std::shared_ptr<CC3DBlendState> CreateBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer);
	virtual std::shared_ptr<CC3DDepthStencilState> CreateDefaultStencilState(bool enableDepthTest, bool enableDepthWrite) ;
	virtual std::shared_ptr<CC3DDepthStencilState> CreateDepthStencilState();
	virtual std::shared_ptr<CC3DSamplerState> CreateSampler(int32_t Mode = CC3DSamplerState::CM_CLAMP,
		int32_t FilterMin = CC3DSamplerState::FM_LINE,
		int32_t FilterMag = CC3DSamplerState::FM_LINE, int32_t FilterMip = CC3DSamplerState::FM_LINE);
	virtual std::shared_ptr<CC3DRasterizerState> CreateRasterizerState(int32_t CullType = CC3DRasterizerState::CT_NONE, bool bWireEnable = false);
	virtual std::shared_ptr<CC3DConstantBuffer> CreateConstantBuffer(int32_t ByteSize);
	virtual std::shared_ptr< CC3DTextureRHI> CreateTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer , int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false);
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromFile(const std::string szFile, bool bGenMipmap = false);
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false);
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromFileCPUAcess(const std::string szFile);
	virtual std::shared_ptr<CC3DRenderTargetRHI> CreateRenderTarget(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> pDestTexture = nullptr, uint8_t format = 0);
	virtual std::shared_ptr<CC3DCubeMapRHI> CreateCubeMap(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget);
	
	virtual void SetBlendState(std::shared_ptr< CC3DBlendState > BlendStateRHI, const float BlendFactor[4], uint32_t SimpleMask);
	virtual void SetDepthStencilState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI, uint32_t StencilRef = 0);
	virtual void SetSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI);
	virtual void SetVSSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI);
	virtual void SetRasterizerState(std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI);
	virtual void UpdateConstantBuffer(std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI,const void* pSrcData);
	virtual void SetVSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI);
	virtual void SetPSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI);
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI);
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI);
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DCubeMapRHI> CubeRHI);
	virtual void SetVSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI);

	virtual bool UpdateTextureInfo(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer, int32_t w, int32_t h);
	virtual bool ReadTextureToCpu(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer);

	virtual void DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI);
	virtual void DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI);
	virtual void DrawPrimitive(const std::array<std::shared_ptr<CC3DVertexBuffer>, 10>& VertexBufferRHI, int32_t VertexBufferCount, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI);

	virtual void GenerateMips(std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI);
	virtual void SetViewPort(float TopLeftX,float TopLeftY,float Width,float Height,float MinDepth=0.0,float MaxDepth=1.0);
};