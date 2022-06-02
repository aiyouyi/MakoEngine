#pragma once
#include "DX11FBO.h"
#include "DX11Context.h"
#include "Toolbox/RectDraw.h"
class DX11DoubleBuffer
{
public:
	DX11DoubleBuffer();
	~DX11DoubleBuffer();
	void Release();

	void InitDoubleBuffer(DX11Texture* pTexIn, DX11Texture* pTexOut, int nWidth, int nHeight);
	void InitDoubleBuffer(int nWidth, int nHeight, bool UseDepth = false, bool bMSAA = false,DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	RectDraw *m_rectDraw;
	bool m_Msaa = false;
public:

	// 绑定FBO A
	void BindFBOA();

	// 绑定FBO B
	void BindFBOB();

	void SetAShaderResource(uint32_t StartSlot);

	void SetBShaderResource(uint32_t StartSlot);

	void BindDoubleBuffer();

	// 交换FBO
	void SwapFBO();

	void SyncAToB();

	//type =0 for (0,1) 
	//type =1 for(-1,1)
	void SyncAToBRegion(float *pVerts,int nVerts,int nStep =2,int type =0);
	
	void SyncBToA();

public:
	DX11Texture* GetFBOTextureA();

	DX11Texture* GetFBOTextureB();

	DX11FBO* GetFBOA();

	DX11FBO* GetFBOB();

	int GetWidth();

	int GetHeight();

private:
	// doublebuffer画布大小
	int m_nWidth;
	int m_nHeight;

	DX11FBO *m_pFBOA;
	DX11FBO *m_pFBOB;

	DX11Texture *m_pTextureA;
	DX11Texture *m_pTextureB;

};

