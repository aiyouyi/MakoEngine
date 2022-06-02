#pragma once
#ifndef _H_MT3D_TEXTURE_H_
#define _H_MT3D_TEXTURE_H_

#include "Common/CC3DUtils.h"
#include"Toolbox/DXUtils/DX11Texture.h"
class CC3DTexture 
{
public:
	CC3DTexture(void);
	virtual ~CC3DTexture(void);
	//初始化
	virtual void Initialize();
	//释放
	virtual void Release();

	bool LoadTexture(const char* filePath, bool useMipmap = false);

	bool LoadTexture(uint8* pSrcRGBA, uint32 width, uint32 height, bool useMipmap = false);

	bool LoadTexture(float r, float g, float b, float a);

	DX11Texture *m_pTexture;
	//当前纹理宽
	uint32 GetTextureWidth();
	//当前纹理高
	uint32 GetTextureHeight();
protected:
	//纹理宽
	uint32 m_nTextureWidth;
	//纹理高
	uint32 m_nTextureHeight;

	DX11Texture *m_pTextureLoad;
};

#endif // _H_MT3D_TEXTURE_H_
