#pragma once
#include "Toolbox/irefptr.h"
#include "Toolbox/vec2.h"
#include <d3d11.h>

class DX11DepthBuffer
{
public:
	DX11DepthBuffer();
	~DX11DepthBuffer();

	bool Create(uint32_t sizeW, uint32_t sizeH, int sampleCount, int quality);
	bool Create32Float(uint32_t sizeW, uint32_t sizeH, bool Msaa);
	IRefPtr<ID3D11DepthStencilView> GetDSV();
	IRefPtr<ID3D11ShaderResourceView> GetDepthSRV();
private:
	IRefPtr<ID3D11DepthStencilView> TexDsv;
	IRefPtr<ID3D11Texture2D> Tex;

	IRefPtr<ID3D11RenderTargetView> RenderTargetView;
	IRefPtr<ID3D11ShaderResourceView> DepthStencilSRV;

	core::vec2u Size;
};