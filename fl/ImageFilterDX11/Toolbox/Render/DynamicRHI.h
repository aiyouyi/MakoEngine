#ifndef CC3D_DYNAMICRHI
#define CC3D_DYNAMICRHI

#include "Toolbox/RenderState/BlendState.h"
#include "Toolbox/RenderState/DepthStencilState.h"
#include "Toolbox/RenderState/SamplerState.h"
#include "Toolbox/RenderState/RasterizerState.h"
#include "DX11ImageFilterDef.h"
#include "Toolbox/inc.h"
#include "CC3DEngine/Common/CC3DUtils.h"

class CC3DIndexBuffer;
class CC3DIndexBuffer;
class CC3DVertexBuffer;
class CC3DConstantBuffer;
class CC3DTextureRHI;
class CC3DRenderTargetRHI;
class CC3DCubeMapRHI;
class DoubleBufferRHI;
class ShaderRHI;
class MaterialTexRHI;
class SwapChainRHI;

class CC3DDynamicRHI
{
public:
	enum RnederAPI
	{
		DX11,
		OPENGL,
	};

	RnederAPI API = DX11;

public:
	CC3DDynamicRHI();
	virtual ~CC3DDynamicRHI();

	virtual std::shared_ptr<CC3DIndexBuffer> CreateIndexBuffer(unsigned short* pData, int nTriangle) = 0;
	virtual std::shared_ptr<CC3DIndexBuffer>  CreateIndexBuffer(unsigned int* pData, int nTriangle) = 0;
	virtual std::shared_ptr<CC3DVertexBuffer> CreateVertexBuffer(float* pData, int nVertex, int nStride) = 0;
	virtual std::shared_ptr<CC3DVertexBuffer> CreateVertexBuffer(int buffercount) { return nullptr; }
	virtual std::shared_ptr<CC3DVertexBuffer> CreateVertexBuffer(float* pVertices, int nVerts, int* pArrange, int nArrange, int nStep) { return nullptr; }
	virtual std::shared_ptr<CC3DVertexBuffer> CreateVertexBuffer(void* pData, int StrideByteWidth, int Count,bool StreamOut) { return nullptr; }
	virtual std::shared_ptr< CC3DBlendState > CreateBlendState(int32_t SrcBlend = CC3DBlendState::BP_SRCALPHA, int32_t DstBlend = CC3DBlendState::BP_INVSRCALPHA) = 0;
	virtual std::shared_ptr<CC3DBlendState> CreateBlendState(bool bBlend, bool bBlendAlpha, bool writeBuffer, bool maskRGB = false) = 0;
	virtual std::shared_ptr<CC3DDepthStencilState> CreateDefaultStencilState(bool enableDepthTest, bool enableDepthWrite) = 0;
	virtual std::shared_ptr<CC3DDepthStencilState> CreateDepthStencilState() = 0;
	virtual std::shared_ptr<CC3DSamplerState> CreateSampler(int32_t Mode = CC3DSamplerState::CM_CLAMP,
															int32_t FilterMin = CC3DSamplerState::FM_LINE,
															int32_t FilterMag = CC3DSamplerState::FM_LINE, int32_t FilterMip = CC3DSamplerState::FM_LINE) = 0;
	virtual std::shared_ptr<CC3DRasterizerState> CreateRasterizerState(int32_t CullType = CC3DRasterizerState::CT_NONE, bool bWireEnable = false) = 0;
	virtual std::shared_ptr<CC3DConstantBuffer> CreateConstantBuffer(int32_t ByteSize) = 0;
	virtual std::shared_ptr< CC3DTextureRHI> CreateTexture() { return nullptr; }
	virtual std::shared_ptr< CC3DTextureRHI> CreateTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer , int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false) = 0;
	virtual std::shared_ptr<CC3DTextureRHI> CreateTexture(float r, float g, float b, float a);
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromFile(const std::string szFile, bool bGenMipmap = false) = 0;
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromZip(void* hZip, const char* szImagePath, bool bGenMipmap = false) = 0;
	virtual std::shared_ptr<CC3DTextureRHI> CreateTextureFromFileCPUAcess(const std::string szFile) = 0;
	virtual std::shared_ptr<MaterialTexRHI> CreateAsynTextureZIP(void* hZip, const std::string& ImagePath, bool bGenMipmap = false);
	virtual std::shared_ptr<MaterialTexRHI> CreateAsynTextureFromFile(const std::string& FileName, bool bGenMipmap =false);
	virtual std::shared_ptr<MaterialTexRHI> CreateAsynTexture(uint32_t format, uint32_t BindFlags, int32_t width, int32_t height, void* pBuffer, int rowBytes = 0, bool bGenMipmap = false, bool bMultSample = false);
	virtual std::shared_ptr<MaterialTexRHI> CreateAsynTexture(float r, float g, float b, float a);
	virtual std::shared_ptr<CC3DRenderTargetRHI> CreateRenderTarget(int width, int height, bool useDepthBuffer = false, std::shared_ptr< CC3DTextureRHI> pDestTexture = nullptr, uint8_t format = 0) = 0;
	virtual std::shared_ptr<CC3DCubeMapRHI> CreateCubeMap(int32_t nTextureWidth, int32_t nTexureHeight, int32_t nMipLevels, bool UseDepth, bool GenMipRetarget) = 0;

	virtual void SetBlendState(std::shared_ptr< CC3DBlendState > BlendStateRHI, const float BlendFactor[4],uint32_t SimpleMask) = 0;
	virtual void SetDepthStencilState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI, uint32_t StencilRef = 0) = 0;
	virtual void SetDepthWritableState(std::shared_ptr<CC3DDepthStencilState> DepthStencilStateRHI) {}
	virtual void SetSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI, int slot = 0) = 0;
	virtual void SetCSSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI, int slot = 0) {};
	virtual void SetVSSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI, int slot = 0) = 0;
	virtual void SetGSSamplerState(std::shared_ptr<CC3DSamplerState> SamplerStateRHI, int slot = 0) {}
	virtual void SetRasterizerState(std::shared_ptr<CC3DRasterizerState> RasterizerStateRHI) = 0;
	virtual void UpdateConstantBuffer(std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI, const void* pSrcData) = 0;
	virtual void SetVSConstantBuffer(uint32_t StartSlot,std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI) = 0;
	virtual void SetPSConstantBuffer(uint32_t StartSlot,std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI) = 0;
	virtual void SetGSConstantBuffer(uint32_t StartSlot, std::shared_ptr<CC3DConstantBuffer> ConstantBufferRHI) {};
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI) = 0;
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI) = 0;
	virtual void SetPSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DCubeMapRHI> TextureRHI) = 0;
	virtual void SetVSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI) = 0;
	virtual void SetGSShaderResource(uint32_t StartSlot, std::shared_ptr<CC3DTextureRHI> TextureRHI) {};

	virtual bool UpdateTextureInfo(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer, int32_t w, int32_t h) = 0;
	virtual bool ReadTextureToCpu(std::shared_ptr<CC3DTextureRHI> TextureRHI, void* pBuffer) = 0;

	virtual void DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI) = 0;
	virtual void DrawPrimitive(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI) = 0;
	virtual void DrawPrimitive(const std::array<std::shared_ptr<CC3DVertexBuffer>, 10>& VertexBufferRHI, int32_t VertexBufferCount, std::shared_ptr<CC3DIndexBuffer> IndexBufferRHI) = 0;
	virtual void DrawPrimitiveTriangles(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI) {}
	virtual void DrawPointList(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI) {}
	virtual void DrawAuto(std::shared_ptr<CC3DVertexBuffer> VertexBufferRHI) {}
	virtual void GenerateMips(std::shared_ptr<CC3DCubeMapRHI> CubeMapRHI) = 0;
	virtual void SetViewPort(float TopLeftX, float TopLeftY, float Width, float Height, float MinDepth = 0.0, float MaxDepth = 1.0) = 0;

	std::shared_ptr< CC3DTextureRHI> FetchTexture(const std::string& szFile, bool bGenMipmap);
	void RecoredTexture(const std::string& szFile, std::shared_ptr< CC3DTextureRHI> TexRHI);

	virtual void SetCullInverse(bool value) = 0 ;
	virtual void SetColorMask(float r, float g, float b, float a) = 0;
	virtual std::shared_ptr< DoubleBufferRHI> CreateDoubleBuffer() = 0;
	virtual std::shared_ptr< ShaderRHI> CreateShaderRHI() = 0;
	virtual std::shared_ptr< SwapChainRHI> CreateSwapChain();
	virtual bool CreateDevice(bool UpdateInternalContext = false) { return false; };
	virtual void CopyResource(std::shared_ptr<CC3DTextureRHI> DstRHI, std::shared_ptr<CC3DTextureRHI> SrcRHI) {};
	virtual void ResolveSubresource(std::shared_ptr<CC3DTextureRHI> DstRHI, std::shared_ptr<CC3DTextureRHI> SrcRHI) {};
	virtual void SOSetTargets(uint32_t Number, std::shared_ptr<CC3DVertexBuffer> SOTarget, uint32_t offsets) {}

private:
	std::map<std::string, std::shared_ptr< CC3DTextureRHI>> m_TexCache;
};

DX11IMAGEFILTER_EXPORTS_API CC3DDynamicRHI::RnederAPI  gRenderAPI ;
DX11IMAGEFILTER_EXPORTS_API CC3DDynamicRHI* GetDynamicRHI();

const float* GetMatData(const glm::mat4& mat);

#endif