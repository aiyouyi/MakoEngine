#include "DX11DoubleBuffer.h"
#include "BaseDefine/Define.h"
#include <algorithm>
#include "DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

extern DWORD ms_dwTextureFormatType[];

DX11DoubleBuffer::DX11DoubleBuffer()
{
	m_pFBOA = nullptr;
	m_pFBOB = nullptr;
	m_nWidth = 0;
	m_nHeight = 0;
	m_pTextureA = nullptr;
	m_pTextureB = nullptr;
	m_rectDraw = NULL;
}


DX11DoubleBuffer::~DX11DoubleBuffer()
{
	this->Release();
}

void DX11DoubleBuffer::Release()
{
	m_pFBOA.reset();
	m_pFBOB.reset();
	m_pTextureA.reset();
	m_pTextureB.reset();
}

void DX11DoubleBuffer::InitDoubleBuffer(std::shared_ptr<CC3DTextureRHI> pTexIn, std::shared_ptr<CC3DTextureRHI> pTexOut, int nWidth, int nHeight, bool bMSAA)
{
	m_Msaa = bMSAA;
	m_pTextureA = pTexIn;
	m_pTextureB = pTexOut;

	m_pFBOA = std::make_shared<DX11FBO>();
	m_pFBOA->initWithTexture(nWidth, nHeight, true, RHIResourceCast(m_pTextureA.get())->GetNativeTex());

	m_pFBOB = std::make_shared<DX11FBO>();
	m_pFBOB->initWithTexture(nWidth, nHeight, true, RHIResourceCast(m_pTextureB.get())->GetNativeTex());

	m_nWidth = nWidth;
	m_nHeight = nHeight;

}

void DX11DoubleBuffer::InitDoubleBuffer( int nWidth, int nHeight, bool UseDepth, bool bMSAA, DXGI_FORMAT format)
{
	m_Msaa = bMSAA;
	m_pFBOA = std::make_shared<DX11FBO>();
	m_pFBOA->initWithTexture(nWidth, nHeight, UseDepth, NULL, format, bMSAA);

	m_pFBOB = std::make_shared<DX11FBO>();
	m_pFBOB->initWithTexture(nWidth, nHeight, UseDepth, NULL, format, bMSAA);

	m_pTextureA = GetDynamicRHI()->CreateTexture();
	RHIResourceCast(m_pTextureA.get())->Attatch(m_pFBOA->getTexture());

	m_pTextureB = GetDynamicRHI()->CreateTexture();
	RHIResourceCast(m_pTextureB.get())->Attatch(m_pFBOB->getTexture());

	m_nWidth = nWidth;
	m_nHeight = nHeight;

}


void DX11DoubleBuffer::InitDoubleBuffer(uint32_t format, int32_t width, int32_t height, bool UseDepth, bool UseMultiTarget, bool bMSAA)
{
	DXGI_FORMAT dxgiFormat = (DXGI_FORMAT)ms_dwTextureFormatType[format];
	InitDoubleBuffer(width, height, UseDepth, bMSAA, dxgiFormat);
}

void DX11DoubleBuffer::BindFBOA()
{
	m_pFBOA->bind();
}

void DX11DoubleBuffer::BindFBOB()
{
	m_pFBOB->bind();
}

void DX11DoubleBuffer::SetAShaderResource(uint32_t StartSlot)
{
	//auto pMaterialView = m_pTextureA->getTexShaderView();
	GetDynamicRHI()->SetPSShaderResource(StartSlot, m_pTextureA);
}

void DX11DoubleBuffer::SetBShaderResource(uint32_t StartSlot)
{
	//auto pMaterialView = m_pTextureB->getTexShaderView();
	GetDynamicRHI()->SetPSShaderResource(StartSlot, m_pTextureB);
}

void DX11DoubleBuffer::BindDoubleBuffer()
{
	auto renderView1 = m_pFBOA->getRenderTargetView();
	auto renderView2 = m_pFBOB->getRenderTargetView();
	ID3D11RenderTargetView *renderView[2] = { renderView1 ,renderView2 };
	DeviceContextPtr->OMSetRenderTargets(2, renderView, m_pFBOA->getDepthStencilView());
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m_nWidth;
	vp.Height = (FLOAT)m_nHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	DeviceContextPtr->RSSetViewports(1, &vp);
}

void DX11DoubleBuffer::SwapFBO()
{
	std::swap(m_pFBOA, m_pFBOB);
	std::swap(m_pTextureA, m_pTextureB);
	//DX11FBO* pTempFBO = m_pFBOA;
	//m_pFBOA = m_pFBOB;
	//m_pFBOB = pTempFBO;
	//DX11Texture *pTempTex = m_pTextureA;
	//m_pTextureA = m_pTextureB;
	//m_pTextureB = pTempTex;
}

void DX11DoubleBuffer::SyncAToB()
{
	if (m_rectDraw == NULL)
	{
		return;
	}
	m_pFBOB->bind();

	m_rectDraw->setShaderTextureView(m_pTextureA);
	m_rectDraw->render();

	//DeviceContextPtr->CopyResource(m_pTextureB->getTex(), );
}

void DX11DoubleBuffer::SyncAToBRegion(float * pVerts, int nVerts, int nStep, int type)
{
	if (m_Msaa)
	{
		DeviceContextPtr->CopyResource(RHIResourceCast(m_pTextureB.get())->GetNativeTex(), RHIResourceCast(m_pTextureA.get())->GetNativeTex());
		return;
	}
	D3D11_BOX box;
	box.front = 0;
	box.back = 1;
	box.left = m_nWidth;
	box.right = 0;
	box.top = m_nHeight;
	box.bottom = 0;

	for (int i=0;i<nVerts;i++)
	{
		float x = pVerts[0];
		float y = pVerts[1];
		pVerts += nStep;

		int xi, yi;
		if (type)
		{
			xi = (x*0.5 + 0.5)*m_nWidth;
			yi = (y*0.5 + 0.5)*m_nHeight;
		}
		else
		{
			xi = x* m_nWidth;
			yi = y * m_nHeight;
		}
		xi = (std::max)(0, xi);
		yi = (std::max)(0, yi);
		box.left = (std::min)(box.left, (UINT)xi);
		box.right = (std::max)(box.right, (UINT)xi);
		box.top = (std::min)(box.top, (UINT)yi);
		box.bottom = (std::max)(box.bottom, (UINT)yi);
	}
	box.right = (std::min)(box.right+1, (UINT)(m_nWidth));
	box.bottom = (std::min)(box.bottom+1, (UINT)(m_nHeight));

	DeviceContextPtr->CopySubresourceRegion(RHIResourceCast(m_pTextureB.get())->GetNativeTex(), 0, box.left, box.top, 0, RHIResourceCast(m_pTextureA.get())->GetNativeTex(), 0, &box);

}

void DX11DoubleBuffer::SyncBToA()
{
	if (m_rectDraw == NULL)
	{
		return;
	}
	m_pFBOA->bind();

	m_rectDraw->setShaderTextureView(m_pTextureB);
	m_rectDraw->render();
}

std::shared_ptr<CC3DTextureRHI> DX11DoubleBuffer::GetFBOTextureA()
{
	return m_pTextureA;
}

std::shared_ptr<CC3DTextureRHI> DX11DoubleBuffer::GetFBOTextureB()
{
	return m_pTextureB;
}

std::shared_ptr<DX11FBO> DX11DoubleBuffer::GetFBOA()
{
	return m_pFBOA;
}

std::shared_ptr<DX11FBO> DX11DoubleBuffer::GetFBOB()
{
	return m_pFBOB;
}

int DX11DoubleBuffer::GetWidth() const
{
	return m_nWidth;
}

int DX11DoubleBuffer::GetHeight() const
{
	return m_nHeight;
}

void DX11DoubleBuffer::ClearA(float r, float g, float b, float a, float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	if (m_pFBOA)
	{
		m_pFBOA->clear(r, g, b, a, depth, Stencil);
	}
}

void DX11DoubleBuffer::ClearDepthA(float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	if (m_pFBOA)
	{
		m_pFBOA->clearDepth(depth, Stencil);
	}
}

void DX11DoubleBuffer::ClearB(float r, float g, float b, float a, float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	if (m_pFBOB)
	{
		m_pFBOB->clear(r, g, b, a, depth, Stencil);
	}
}

void DX11DoubleBuffer::ClearDepthB(float depth /*= 1.0f*/, unsigned char Stencil /*= 0*/)
{
	if (m_pFBOB)
	{
		m_pFBOB->clearDepth(depth, Stencil);
	}
}
