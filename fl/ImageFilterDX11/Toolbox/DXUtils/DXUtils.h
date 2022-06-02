#pragma once

#include "DX11Shader.h"
#include "DX11FBO.h"
#include "DX11Texture.h"
#include "DX11Context.h"
#include "Toolbox/zip/unzip.h"
#include "common.h"
#include "CC3DEngine/Common/CC3DUtils.h"

class DXUtils
{
public:
	static ID3D11SamplerState* SetSampler(D3D11_TEXTURE_ADDRESS_MODE mode = D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR)
	{
		//创建纹理采样
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = filter;
		sampDesc.AddressU = mode;
		sampDesc.AddressV = mode;
		sampDesc.AddressW = mode;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		ID3D11SamplerState* pSamplerLinear;
		HRESULT hr = DevicePtr->CreateSamplerState(&sampDesc, &pSamplerLinear);
		if (FAILED(hr))
		{
			return NULL;
		}
		else
		{
			return pSamplerLinear;
		}
	}

	static ID3D11BlendState * CreateBlendState(D3D11_BLEND SrcBlend = D3D11_BLEND_SRC_ALPHA, D3D11_BLEND DstBlend = D3D11_BLEND_INV_SRC_ALPHA)
	{
		D3D11_BLEND_DESC blend_desc;
		memset(&blend_desc, 0, sizeof(blend_desc));
		blend_desc.AlphaToCoverageEnable = false;
		blend_desc.IndependentBlendEnable = false;
		blend_desc.RenderTarget[0].BlendEnable = true;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

		blend_desc.RenderTarget[0].SrcBlend = SrcBlend;
		blend_desc.RenderTarget[0].DestBlend = DstBlend;

		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ID3D11BlendState *pBlendState = NULL;
		DevicePtr->CreateBlendState(&blend_desc, &pBlendState);
		return pBlendState;

	}

	static ID3D11BlendState * CloseBlendState()
	{
		D3D11_BLEND_DESC blend_desc;
		memset(&blend_desc, 0, sizeof(blend_desc));
		blend_desc.AlphaToCoverageEnable = false;
		blend_desc.IndependentBlendEnable = false;
		blend_desc.RenderTarget[0].BlendEnable = false;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;



		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ID3D11BlendState *pBlendState = NULL;
		DevicePtr->CreateBlendState(&blend_desc, &pBlendState);
		return pBlendState;

	}


	static void CompileShaderWithFile(DX11Shader *pShader, char *pFileName, CCVetexAttribute *pAttribute, int nAttri)
	{
		pShader->SetAttribute(pAttribute, nAttri);
		pShader->initShaderWithFile(pFileName);
	}

	static ID3D11Buffer * CreateVertexBuffer(float *pData, int nVertex, int nStride)
	{
		//创建顶点缓存
		D3D11_BUFFER_DESC verBufferDesc;
		memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		verBufferDesc.ByteWidth = sizeof(float) * nStride * nVertex;
		verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		verBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		D3D11_SUBRESOURCE_DATA vertexInitData;
		memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexInitData.pSysMem = pData;
		ID3D11Buffer *pVertexBuffer;
		HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &pVertexBuffer);
		if (FAILED(hr))
		{
			return NULL;
		}
		else
		{
			return pVertexBuffer;
		}
	}

	static ID3D11Buffer * CreateIndexBuffer(unsigned short *pData, int nTriangle)
	{
		//创建索引buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		indexBufferDesc.ByteWidth = sizeof(unsigned short) * nTriangle * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA indexInitData;
		memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		indexInitData.pSysMem = pData;
		ID3D11Buffer *pIndexBuffer;
		HRESULT hr = DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &pIndexBuffer);
		if (FAILED(hr))
		{
			return NULL;
		}
		else
		{
			return pIndexBuffer;
		}
	}

	static ID3D11Buffer * CreateIndexBuffer(unsigned int *pData, int nTriangle)
	{
		//创建索引buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		indexBufferDesc.ByteWidth = sizeof(unsigned int) * nTriangle * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA indexInitData;
		memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		indexInitData.pSysMem = pData;
		ID3D11Buffer *pIndexBuffer;
		HRESULT hr = DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &pIndexBuffer);
		if (FAILED(hr))
		{
			return NULL;
		}
		else
		{
			return pIndexBuffer;
		}
	}

	static ID3D11Buffer * CreateConstantBuffer( int ByteSize)
	{
		//创建索引buffer
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.ByteWidth = ByteSize;
		ID3D11Buffer* pConstantBuffer;
		HRESULT hr = DevicePtr->CreateBuffer(&bd, NULL, &pConstantBuffer);
		if (FAILED(hr))
		{
			return NULL;
		}
		else
		{
			return pConstantBuffer;
		}
	}

	static void UpdateVertexBuffer(ID3D11Buffer *pVertexBuffer,float  *pData, int nVertex, int perUpdateSize, int sizePerVertex, int begin=0, int offset=0)
	{
		if (nVertex <= 0 || nullptr == pData) 
			return ;
		if (begin < 0)
		{
			begin = 0;
		}

		D3D11_MAPPED_SUBRESOURCE mapSubResource;
		HRESULT hr = DeviceContextPtr->Map(pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapSubResource);
		if (FAILED(hr))
		{
			return ;
		}

		if (perUpdateSize == sizePerVertex && offset == 0)
		{
			memcpy((unsigned char *)mapSubResource.pData + begin * sizePerVertex, pData, nVertex*sizePerVertex);
		}

		else
		{
			unsigned char *pDest = ((unsigned char *)mapSubResource.pData) + begin * sizePerVertex + offset;
			unsigned char* pSrc = (unsigned char *)pData;
			for (int i = 0; i < nVertex; ++i)
			{
				memcpy(pDest, pSrc, perUpdateSize);
				pDest += sizePerVertex;
				pSrc += perUpdateSize;
			}
		}
		DeviceContextPtr->Unmap(pVertexBuffer, 0);
	}

	static void SetViewPort(int nWidth, int nHeight, int x = 0, int y = 0)
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)nWidth;
		vp.Height = (FLOAT)nHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = x;
		vp.TopLeftY = y;
		DeviceContextPtr->RSSetViewports(1, &vp);
	}

	static DX11Texture *CreateTexFromFile(const char *szImagePath, bool bGenMipmap)
	{
		DX11Texture *pTex = NULL;
		int index;
		ZIPENTRY ze;

		int texW, texH, nChannel;
		unsigned char *pImageDataSrc = ccLoadImage(szImagePath, &texW, &texH, &nChannel, 0);
		if (pImageDataSrc != NULL && texW*texH > 0)
		{
			unsigned char *pImageDataDest = pImageDataSrc;
			//转换成4通道.
			if (nChannel == 3)
			{
				pImageDataDest = new unsigned char[texW*texH * 4];
				unsigned char *pImageSourceTemp = pImageDataSrc;
				unsigned char *pImageDestTemp = pImageDataDest;
				for (int r = 0; r < texH; ++r)
				{
					for (int c = 0; c < texW; ++c)
					{
						pImageDestTemp[0] = pImageSourceTemp[0];
						pImageDestTemp[1] = pImageSourceTemp[1];
						pImageDestTemp[2] = pImageSourceTemp[2];
						pImageDestTemp[3] = 255;

						pImageSourceTemp += 3;
						pImageDestTemp += 4;
					}
				}

				delete[]pImageDataSrc;
			}
			pTex = new DX11Texture();
			pTex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, texW, texH, pImageDataDest, texW * 4, bGenMipmap);
			delete[]pImageDataDest;
		}
		return pTex;
	}

	static DX11Texture *CreateTexFromZIP(HZIP hZip, const char *szImagePath, bool bGenMipmap)
	{
		DX11Texture *pTex = NULL;
		int index;
		ZIPENTRY ze;

		if (FindZipItem(hZip, szImagePath, true, &index, &ze) == ZR_OK)
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			if (res == ZR_OK)
			{
				int texW, texH, nChannel;
				unsigned char *pImageDataSrc = ccLoadImageFromBuffer((unsigned char *)pDataBuffer, ze.unc_size, &texW, &texH, &nChannel, 0);
				if (pImageDataSrc != NULL && texW*texH > 0)
				{
					unsigned char *pImageDataDest = pImageDataSrc;
					//转换成4通道.
					if (nChannel == 3)
					{
						pImageDataDest = new unsigned char[texW*texH * 4];
						unsigned char *pImageSourceTemp = pImageDataSrc;
						unsigned char *pImageDestTemp = pImageDataDest;
						for (int r = 0; r < texH; ++r)
						{
							for (int c = 0; c < texW; ++c)
							{
								pImageDestTemp[0] = pImageSourceTemp[0];
								pImageDestTemp[1] = pImageSourceTemp[1];
								pImageDestTemp[2] = pImageSourceTemp[2];
								pImageDestTemp[3] = 255;

								pImageSourceTemp += 3;
								pImageDestTemp += 4;
							}
						}

						delete[]pImageDataSrc;
					}
					pTex = new DX11Texture();
					pTex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, texW, texH, pImageDataDest, texW * 4, bGenMipmap);
					delete[]pImageDataDest;
				}
			}

			delete[]pDataBuffer;
		}

		return pTex;
	}

	static float *CreateHDRFromZIP(HZIP hZip, const char *szImagePath, int &nWidth, int &nHeight)
	{
		int index;
		ZIPENTRY ze;

		if (FindZipItem(hZip, szImagePath, true, &index, &ze) == ZR_OK)
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			float *pImageDataDest = NULL;
			if (res == ZR_OK)
			{
				int width, height, nrComponents;
				pImageDataDest = ccLoadImagefFromBuffer((unsigned char *)pDataBuffer, ze.unc_size, &width, &height, &nrComponents, 0);
				nWidth = width;
				nHeight = height;
			}
			delete[]pDataBuffer;
			return pImageDataDest;
		}
		return NULL;
	}



	static void  SetRasterizerState(bool FillSolid = true)
	{
		D3D11_RASTERIZER_DESC rasterizer_desc;
		memset(&rasterizer_desc, 0, sizeof(D3D11_RASTERIZER_DESC));
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		if (FillSolid)
		{
			rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		}
		else
		{
			rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
		}
		
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.FrontCounterClockwise = true;
		ID3D11RasterizerState *pRasterizerState = NULL;
		DevicePtr->CreateRasterizerState(&rasterizer_desc, &pRasterizerState);
		DeviceContextPtr->RSSetState(pRasterizerState);
		pRasterizerState->Release();
	}

	static BYTE *CreateImgFromZIP(HZIP hZip, const char *szImagePath,int &nWidth,int &nHeight)
	{
		int index;
		ZIPENTRY ze;

		if (FindZipItem(hZip, szImagePath, true, &index, &ze) == ZR_OK)
		{
			char *pDataBuffer = new char[ze.unc_size];
			ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
			unsigned char *pImageDataDest = NULL;
			if (res == ZR_OK)
			{
				int texW, texH, nChannel;
				unsigned char *pImageDataSrc = ccLoadImageFromBuffer((unsigned char *)pDataBuffer, ze.unc_size, &texW, &texH, &nChannel, 0);
				nWidth = texW;
				nHeight = texH;
				if (pImageDataSrc != NULL && texW*texH > 0)
				{
					pImageDataDest = pImageDataSrc;
					//转换成4通道.
					if (nChannel == 3)
					{
						pImageDataDest = new unsigned char[texW*texH * 4];
						unsigned char *pImageSourceTemp = pImageDataSrc;
						unsigned char *pImageDestTemp = pImageDataDest;
						for (int r = 0; r < texH; ++r)
						{
							for (int c = 0; c < texW; ++c)
							{
								pImageDestTemp[0] = pImageSourceTemp[0];
								pImageDestTemp[1] = pImageSourceTemp[1];
								pImageDestTemp[2] = pImageSourceTemp[2];
								pImageDestTemp[3] = 255;

								pImageSourceTemp += 3;
								pImageDestTemp += 4;
							}
						}

						delete[]pImageDataSrc;
					}
					
				}
			}
			delete[]pDataBuffer;
			return pImageDataDest;
		}
		return NULL;
	}

	static BYTE *CreateImgFromFile(const char *szImagePath, int &nWidth, int &nHeight)
	{
		int texW, texH, nChannel;
		unsigned char *pImageDataSrc = ccLoadImage(szImagePath, &texW, &texH, &nChannel, 0);

		nWidth = texW;
		nHeight = texH;
		if (pImageDataSrc != NULL && texW*texH > 0)
		{
			unsigned char *pImageDataDest = pImageDataSrc;
			//转换成4通道.
			if (nChannel == 3)
			{
				pImageDataDest = new unsigned char[texW*texH * 4];
				unsigned char *pImageSourceTemp = pImageDataSrc;
				unsigned char *pImageDestTemp = pImageDataDest;
				for (int r = 0; r < texH; ++r)
				{
					for (int c = 0; c < texW; ++c)
					{
						pImageDestTemp[0] = pImageSourceTemp[0];
						pImageDestTemp[1] = pImageSourceTemp[1];
						pImageDestTemp[2] = pImageSourceTemp[2];
						pImageDestTemp[3] = 255;

						pImageSourceTemp += 3;
						pImageDestTemp += 4;
					}
				}
				delete[]pImageDataSrc;
			}
			return pImageDataDest;
		}
		
		return NULL;
	}

	static bool ReadTextureToCpu(ID3D11Texture2D *Tex, void* pBuffer,uint32_t width,uint32_t height)
	{
		if (Tex != NULL && pBuffer != NULL)
		{
			D3D11_TEXTURE2D_DESC texDesc;
			Tex->GetDesc(&texDesc);
			texDesc.MipLevels = 1;
			texDesc.Usage = D3D11_USAGE_STAGING;
			texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			texDesc.BindFlags = 0;
			texDesc.SampleDesc.Count = 1;
			ID3D11Texture2D* TextureTmp = NULL;
			DevicePtr->CreateTexture2D(&texDesc, NULL, &TextureTmp);


			DeviceContextPtr->CopyResource(TextureTmp, Tex);

			D3D11_MAPPED_SUBRESOURCE mapData;
			// 		mapData.pData = pBuffer;
			// 		mapData.RowPitch = m_nWidth * 4;
			DeviceContextPtr->Map(TextureTmp, 0, D3D11_MAP_READ, 0, &mapData);
			DeviceContextPtr->Unmap(TextureTmp, 0);

			char* pdata = static_cast<char*>(mapData.pData);
			char* pbuf = static_cast<char*>(pBuffer);
			for (int i = 0; i < height; i++) {
				memcpy(pbuf, pdata, width * 4);
				pbuf += width * 4;
				pdata += mapData.RowPitch;
			}

			TextureTmp->Release();
		}

		return false;
	}
};
