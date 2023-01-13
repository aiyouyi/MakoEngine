#pragma once
#include "DX11FBO.h"
#include "DX11Context.h"
#include "Toolbox/RectDraw.h"
#include "Toolbox/Render/DoubleBufferRHI.h"
#include "DX11Texture2D.h"

class DX11DoubleBuffer : public DoubleBufferRHI
{
public:
	DX11DoubleBuffer();
	~DX11DoubleBuffer();
	void Release();

	virtual void InitDoubleBuffer(std::shared_ptr<CC3DTextureRHI> pTexIn, std::shared_ptr<CC3DTextureRHI> pTexOut, int nWidth, int nHeight, bool bMSAA =false) override;
	virtual void InitDoubleBuffer(uint32_t format, int32_t width, int32_t height, bool UseDepth, bool UseMultiTarget, bool bMSAA) override;

	void InitDoubleBuffer(int nWidth, int nHeight, bool UseDepth = false, bool bMSAA = false,DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	RectDraw *m_rectDraw;

public:

	// 绑定FBO A
	void BindFBOA() override;

	// 绑定FBO B
	void BindFBOB() override;

	void SetAShaderResource(uint32_t StartSlot);
	void SetBShaderResource(uint32_t StartSlot);
	void BindDoubleBuffer() override;

	// 交换FBO
	void SwapFBO() override;
	void SyncAToB() override;

	//type =0 for (0,1) 
	//type =1 for(-1,1)
	void SyncAToBRegion(float *pVerts,int nVerts,int nStep =2,int type =0) override;
	
	void SyncBToA() override;
	bool IsMsaa() const { return m_Msaa; }

	int GetWidth() const override;
	int GetHeight() const override;

	void ClearA(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0) override;
	void ClearDepthA(float depth = 1.0f, unsigned char Stencil = 0) override;

	void ClearB(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0) override;
	void ClearDepthB(float depth = 1.0f, unsigned char Stencil = 0) override;

public:
	std::shared_ptr<CC3DTextureRHI> GetFBOTextureA();
	std::shared_ptr<CC3DTextureRHI> GetFBOTextureB();
	std::shared_ptr<DX11FBO> GetFBOA();
	std::shared_ptr <DX11FBO> GetFBOB();

private:
	// doublebuffer画布大小
	int m_nWidth;
	int m_nHeight;
	bool m_Msaa = false;

	std::shared_ptr<DX11FBO> m_pFBOA;
	std::shared_ptr<DX11FBO> m_pFBOB;

	std::shared_ptr<CC3DTextureRHI> m_pTextureA;
	std::shared_ptr<CC3DTextureRHI> m_pTextureB;

};

