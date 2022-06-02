#include "DX11Texture.h"
#include "DX11Context.h"
#include "common.h"
#include "Toolbox/Helper.h"
#include <d3dx11.h>  
#include "DDS/DDSTextureLoader.h"
#include "CC3DEngine/Common/CC3DUtils.h"
#include "Toolbox/DXUtils/DXUtils.h"

DX11Texture::DX11Texture()
{
	m_bGenMipmap = false;
	m_texShaderView = NULL;
	m_pTexture = NULL;

	m_nWidth = 0;
	m_nHeight = 0;
}


DX11Texture::~DX11Texture()
{
	destory();
}

bool DX11Texture::initTexture(DXGI_FORMAT format, UINT BindFlags, int width, int height, void *pBuffer, int rowBytes, bool bGenMipmap, bool bMultSample)
{
	destory();

	m_nWidth = width;
	m_nHeight = height;
	m_bGenMipmap = bGenMipmap;
	if (width*height == 0) { return false; }

	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = format;
	if (bGenMipmap)
	{
		texDesc.MipLevels = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	else
	{
		texDesc.MipLevels = 1;
	}
	texDesc.ArraySize = 1;
	texDesc.CPUAccessFlags = 0;

	if ((BindFlags & D3D11_BIND_RENDER_TARGET) != 0 && bMultSample)
	{
		UINT qLevels;
		DevicePtr->CheckMultisampleQualityLevels(format, 4, &qLevels);
		texDesc.SampleDesc.Count = 4;
		texDesc.SampleDesc.Quality = qLevels > 4? 4: qLevels-1;
	}
	else
	{
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
	}
	
	
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	if (bGenMipmap)
	{
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	}
	else
	{
		texDesc.BindFlags = BindFlags ;
	}
	

	D3D11_SUBRESOURCE_DATA *pInitData = NULL;

	if (bGenMipmap)
	{
		hr = DevicePtr->CreateTexture2D(&texDesc, pInitData, &m_pTexture);
		if (FAILED(hr))
		{
			return false;
		}
		if (pBuffer != NULL)
		{
			DeviceContextPtr->UpdateSubresource(m_pTexture, 0, NULL, pBuffer, rowBytes, 0);
		}
	}
	else
	{
		D3D11_SUBRESOURCE_DATA initData;
		if (pBuffer != 0)
		{
			initData.pSysMem = pBuffer;
			initData.SysMemPitch = rowBytes;
			initData.SysMemSlicePitch = height * rowBytes;
			pInitData = &initData;
		}

		hr = DevicePtr->CreateTexture2D(&texDesc, pInitData, &m_pTexture);
		if (FAILED(hr))
		{
			return false;
		}
	}
	

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	memset(&srvd, 0, sizeof(srvd));
	srvd.Format = texDesc.Format;
	if (texDesc.SampleDesc.Count > 1)
	{
		srvd.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		srvd.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	}
	srvd.Texture2D.MipLevels = -1;
	srvd.Texture2D.MostDetailedMip = 0;
	hr = DevicePtr->CreateShaderResourceView(m_pTexture, &srvd, &m_texShaderView);
	if (FAILED(hr))
	{
		m_pTexture->Release();
		m_pTexture = NULL;

		m_texShaderView = NULL;
		return false;
	}
	if (bGenMipmap)
	{
		DeviceContextPtr->GenerateMips(m_texShaderView);
	}

	m_nWidth = width;
	m_nHeight = height;
	return true;
}

bool DX11Texture::initTextureFromFile(std::string szFile, bool bGenMipmap)
{
	destory();

	//int nLen = MultiByteToWideChar(CP_ACP, 0, szFile.c_str(), -1, NULL, 0);
	//wchar_t wszFile[256];
	//MultiByteToWideChar(CP_ACP, 0, szFile.c_str(), -1, wszFile, nLen);

	HRESULT hr = D3DX11CreateTextureFromFile(DevicePtr, szFile.c_str(), NULL, NULL, (ID3D11Resource **)&m_pTexture, NULL);
	if (FAILED(hr))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC texDesc;
	m_pTexture->GetDesc(&texDesc);

	m_nWidth = texDesc.Width;
	m_nHeight = texDesc.Height;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	memset(&srvd, 0, sizeof(srvd));
	srvd.Format = texDesc.Format;
	srvd.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MipLevels = -1;
	srvd.Texture2D.MostDetailedMip = 0;
	hr = DevicePtr->CreateShaderResourceView(m_pTexture, &srvd, &m_texShaderView);
	if (FAILED(hr))
	{
		m_pTexture->Release();
		m_pTexture = NULL;

		m_texShaderView = NULL;
		return false;
	}

	return true;
}

bool DX11Texture::initTextureFromZip(HZIP hZip, const char *szImagePath, bool bGenMipmap)
{
	int index;
	ZIPENTRY ze;

	if (FindZipItem(hZip, szImagePath, true, &index, &ze) == ZR_OK)
	{
		char *pDataBuffer = new char[ze.unc_size];
		ZRESULT res = UnzipItem(hZip, index, pDataBuffer, ze.unc_size);
		if (res == ZR_OK)
		{
			if(strcmp(szImagePath + strlen(szImagePath) - 4,".dds") == 0)
			{
				destory();
				SDDSInfo ddsInfo;
				CreateDDSTextureFromBuffer(DevicePtr, (BYTE*)pDataBuffer, ze.unc_size, &m_texShaderView, &ddsInfo);

				m_nWidth = ddsInfo.iWidth;
				m_nHeight = ddsInfo.iHeight;
				m_bGenMipmap = ddsInfo.iMipCount>1;
			}
			else
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
					initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, texW, texH, pImageDataDest, texW * 4, bGenMipmap);
					delete[]pImageDataDest;
				}
			}
		}

		delete[]pDataBuffer;
	}

	return true;
}

bool DX11Texture::initTextureFromFileCPUAcess(std::string path)
{
	int w, h, ch;
	const char* filename = path.c_str();
	unsigned char *pImageDataSrc = ccLoadImage(filename, &w, &h, &ch, 0);
	if (pImageDataSrc != NULL && w*h > 0)
	{
		unsigned char *pImageDataDest = pImageDataSrc;
		//转换成4通道.
		if (ch == 3)
		{
			pImageDataDest = new unsigned char[w*h * 4];
			unsigned char *pImageSourceTemp = pImageDataSrc;
			unsigned char *pImageDestTemp = pImageDataDest;
			for (int r = 0; r < h; ++r)
			{
				for (int c = 0; c < w; ++c)
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
		initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, w, h, pImageDataDest, w * 4, false);
		delete[] pImageDataDest;
		return true;
	}
	return false;
}

bool DX11Texture::updateTextureInfo(void *pBuffer, int w, int h)
{
	if (m_pTexture != NULL && pBuffer != NULL)
	{
		DeviceContextPtr->UpdateSubresource(m_pTexture, 0, NULL, pBuffer, w, 0);
		return true;
	}
	return false;
}

bool DX11Texture::ReadTextureToCpu(void * pBuffer)
{
	return DXUtils::ReadTextureToCpu(m_pTexture, pBuffer, m_nWidth, m_nHeight);
}

ID3D11Texture2D *DX11Texture::getTex()
{
	return m_pTexture;
}

ID3D11ShaderResourceView *DX11Texture::getTexShaderView()
{
	return m_texShaderView;
}

int DX11Texture::width()
{
	return m_nWidth;
}
int DX11Texture::height()
{
	return m_nHeight;
}

void DX11Texture::destory()
{
	m_nWidth = 0;
	m_nHeight = 0;

	SAFERALEASE(m_texShaderView);
	SAFERALEASE(m_pTexture);
}