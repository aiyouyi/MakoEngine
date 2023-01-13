#pragma once
#ifndef __DX11TEXTURE_H__
#define __DX11TEXTURE_H__

#include <d3d11.h>
#include <string>
#include "Toolbox/zip/unzip.h"
#include "IReferenceCounted.h"

class DX11IMAGEFILTER_EXPORTS_CLASS DX11Texture: public IReferenceCounted
{
public:
	DX11Texture();
	virtual ~DX11Texture();

	bool initTexture(DXGI_FORMAT format, UINT BindFlags, int width, int height, void *pBuffer = NULL, int rowBytes = 0, bool bGenMipmap = false, bool bMultSample=false);
	bool initTextureFromFile(std::string szFile, bool bGenMipmap = false);
	bool initTextureFromZip(HZIP hZip, const char *szImagePath, bool bGenMipmap=false);
	bool initTextureFromFileCPUAcess(std::string szFile);

	bool updateTextureInfo(void *pBuffer, int w, int h);

	bool ReadTextureToCpu(void *pBuffer);

	ID3D11Texture2D *getTex();
	ID3D11ShaderResourceView *getTexShaderView();

	int width();
	int height();

	void destory();
	bool m_bGenMipmap;
	ID3D11ShaderResourceView *m_texShaderView;
	int m_nWidth;
	int m_nHeight;
private:
	ID3D11Texture2D *m_pTexture;



};

#endif