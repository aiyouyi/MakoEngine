#ifndef CC3D_DYNAMICRHI
#define CC3D_DYNAMICRHI

#include "Toolbox/RenderState/BlendState.h"
#include "Toolbox/RenderState/DepthStencilState.h"
#include "Toolbox/RenderState/SamplerState.h"
#include "Toolbox/RenderState/RasterizerState.h"

class CC3DIndexBuffer;
class CC3DIndexBuffer;
class CC3DVertexBuffer;
class CC3DConstantBuffer;
class CC3DTextureRHI;
class CC3DRenderTargetRHI;
class CC3DCubeMapRHI;

class CC3DDynamicRHI
{
public:
	enum	//Surface Format Type
	{
		SFT_A8R8G8B8,
		SFT_A8B8G8R8,
		SFT_X8R8G8B8,
		SFT_D16,
		SFT_A32R32G32B32F,
		SFT_R32G32B32F,
		SFT_A16R16G16B16F,
		SFT_G16R16F,
		SFT_R16F,
		SFT_R32F,
		SFT_UNKNOWN,
		SFT_D24S8,
		SFT_R32G32F,
		SFT_R5G6B5,
		SFT_A8,
		SFT_R16G16,
		SFT_A16B16G16R16,
		SFT_R8,
		SFT_A8R8G8B8_S,
		SFT_G16R16_S,
		SFT_R16,
		SFT_A16B16G16R16_S,
		//Compress Type
		SFT_BC1,
		SFT_BC2,
		SFT_BC3,
		SFT_BC4,
		SFT_BC5,
		SFT_MAX
	};
public:
	CC3DDynamicRHI();
	virtual ~CC3DDynamicRHI();

	virtual std::shared_ptr<CC3DIndexBuffer> CreateIndexBuffer(unsigned short* pData, int nTriangle) = 0;
	virtual std::shared_ptr<CC3DIndexBuffer>  CreateIndexBuffer(unsigned int* pData, int nTriangle) = 0;
	virtual std::shared_ptr<CC3DVertexBuffer> CreateVertexBuffer(float* pData, int nVertex, int nStride) = 0;
	virtual std::shared_ptr< CC3DBlendState > CreateBlendState(int32_t SrcBlend = CC3DBlendState::BP_SRCALPHA, int32_t DstBlend = CC3DBlendState::BP_INVSRCALPHA) = 0;
	virtual std::shared_ptr<CC3DBlendState> CreateBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer) = 0;
	virtual std::shared_ptr<CC3DDepthStencilState> CreateDefaultStencilState(bool enableDepthTest, bool enableDepthWrite) = 0;
	virtual std::shared_ptr<CC3DDepthStencilState> CreateDepthStencilState() = 0;
	virtual std::shared_ptr<CC3DSamplerState> CreateSampler(int32_t Mode = CC3DSamplerState::CM_CLAMP,
															int32_t FilterMin = CC3DSamplerState::FM_LINE,
															int32_t FilterMag = CC3DSamplerState::FM_LINE, int32_t FilterMip = CC3DSamplerState::FM_LINE) = 0;
	virtual std::shared_ptr<CC3DRasterizerState> CreateRasterizerState(int32_t CullType = CC3DRasterizerState::CT_NONE, bool bWireEnable = false) = 0;
	virtual std::shared_ptr<CC3DConstantBuffer> CreateConstantBuffer(int32_t ByteSize) = 0;
	virtual std::shared_ptr< CC3DTextureRHI> CreateTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer , int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false) = 0;
	virtual std::shared_ptr<CC3DTextureRHI> CreateTexture(float r, float g, float b, float a);
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromFile(const std::string szFile, bool bGenMipmap = false) = 0;
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false) = 0;
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromFileCPUAcess(const std::string szFile) = 0;
	virtual std::shared_ptr<CC3DRenderTargetRHI> CreateRenderTarget(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> pDestTexture = nullptr, uint8_t format = 0) = 0;
	virtual std::shared_ptr<CC3DCubeMapRHI> CreateCubeMap(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget) = 0;

	virtual void SetBlendState(std::shared_ptr< CC3DBlendState > BlendStateRHI, const float BlendFactor[4],uint32_t SimpleMask) = 0;
	virtual void SetDepthStencilState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI, uint32_t StencilRef = 0) = 0;
	virtual void SetSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI) = 0;
	virtual void SetVSSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI) = 0;
	virtual void SetRasterizerState(std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI) = 0;
	virtual void UpdateConstantBuffer(std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI, const void* pSrcData) = 0;
	virtual void SetVSConstantBuffer(uint32_t StartSlot,std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI) = 0;
	virtual void SetPSConstantBuffer(uint32_t StartSlot,std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI) = 0;
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI) = 0;
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI) = 0;
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DCubeMapRHI> TextureRHI) = 0;
	virtual void SetVSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI) = 0;

	virtual bool UpdateTextureInfo(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer, int32_t w, int32_t h) = 0;
	virtual bool ReadTextureToCpu(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer) = 0;

	virtual void DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI) = 0;
	virtual void DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI) = 0;
	virtual void DrawPrimitive(const std::array<std::shared_ptr<CC3DVertexBuffer>, 10>& VertexBufferRHI, int32_t VertexBufferCount, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI) = 0;
	virtual void GenerateMips(std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI) = 0;
	virtual void SetViewPort(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth = 0.0, float MaxDepth = 1.0) = 0;

	std::shared_ptr< CC3DTextureRHI> FetchTexture(const std::string& szFile, bool bGenMipmap);
	void RecoredTexture(const std::string& szFile, std::shared_ptr< CC3DTextureRHI> TexRHI);


private:
	std::map<std::string, std::shared_ptr< CC3DTextureRHI>> m_TexCache;
};

CC3DDynamicRHI* GetDynamicRHI();

#endif