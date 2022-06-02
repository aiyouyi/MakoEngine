#include "DX11DoubleBuffer.h"
#include "BaseDefine/Define.h"
#include <algorithm>

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
	SAFE_DELETE(m_pFBOA);
	SAFE_DELETE(m_pFBOB);
//	SAFE_DELETE(m_pTextureA);
//	SAFE_DELETE(m_pTextureB);
}

void DX11DoubleBuffer::InitDoubleBuffer(DX11Texture * pTexIn, DX11Texture * pTexOut, int nWidth, int nHeight)
{
	SAFE_DELETE(m_pFBOA);
	SAFE_DELETE(m_pFBOB);
	m_pTextureA = pTexIn;
	m_pTextureB = pTexOut;

	m_pFBOA = new DX11FBO();
	m_pFBOA->initWithTexture(nWidth, nHeight, true, m_pTextureA->getTex());

	m_pFBOB = new DX11FBO();
	m_pFBOB->initWithTexture(nWidth, nHeight, true, m_pTextureB->getTex());

	m_nWidth = nWidth;
	m_nHeight = nHeight;

}

void DX11DoubleBuffer::InitDoubleBuffer( int nWidth, int nHeight, bool UseDepth, bool bMSAA, DXGI_FORMAT format)
{
	SAFE_DELETE(m_pFBOA);
	SAFE_DELETE(m_pFBOB);

	m_pFBOA = new DX11FBO();
	m_pFBOA->initWithTexture(nWidth, nHeight, UseDepth, NULL, format, bMSAA);

	m_pFBOB = new DX11FBO();
	m_pFBOB->initWithTexture(nWidth, nHeight, UseDepth, NULL, format, bMSAA);

	m_pTextureA = m_pFBOA->getTexture();
	m_pTextureB = m_pFBOB->getTexture();
	m_nWidth = nWidth;
	m_nHeight = nHeight;

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
	auto pMaterialView = m_pTextureA->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(StartSlot, 1, &pMaterialView);
}

void DX11DoubleBuffer::SetBShaderResource(uint32_t StartSlot)
{
	auto pMaterialView = m_pTextureB->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(StartSlot, 1, &pMaterialView);
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
	DX11FBO* pTempFBO = m_pFBOA;
	m_pFBOA = m_pFBOB;
	m_pFBOB = pTempFBO;
	DX11Texture *pTempTex = m_pTextureA;
	m_pTextureA = m_pTextureB;
	m_pTextureB = pTempTex;
}

void DX11DoubleBuffer::SyncAToB()
{
	if (m_rectDraw == NULL)
	{
		return;
	}
	m_pFBOB->bind();

	m_rectDraw->setShaderTextureView(m_pTextureA->getTexShaderView());
	m_rectDraw->render();

	//DeviceContextPtr->CopyResource(m_pTextureB->getTex(), );
}

void DX11DoubleBuffer::SyncAToBRegion(float * pVerts, int nVerts, int nStep, int type)
{
	if (m_Msaa)
	{
		DeviceContextPtr->CopyResource(m_pTextureB->getTex(), m_pTextureA->getTex());
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

	DeviceContextPtr->CopySubresourceRegion(m_pTextureB->getTex(), 0, box.left, box.top, 0, m_pTextureA->getTex(), 0, &box);

}

void DX11DoubleBuffer::SyncBToA()
{
	if (m_rectDraw == NULL)
	{
		return;
	}
	m_pFBOA->bind();

	m_rectDraw->setShaderTextureView(m_pTextureB->getTexShaderView());
	m_rectDraw->render();
//	DeviceContextPtr->CopyResource(m_pTextureA->getTex(), m_pTextureB->getTex());
}

DX11Texture * DX11DoubleBuffer::GetFBOTextureA()
{
	return m_pTextureA;
}

DX11Texture * DX11DoubleBuffer::GetFBOTextureB()
{
	return m_pTextureB;
}

DX11FBO * DX11DoubleBuffer::GetFBOA()
{
	return m_pFBOA;
}

DX11FBO * DX11DoubleBuffer::GetFBOB()
{
	return m_pFBOB;
}

int DX11DoubleBuffer::GetWidth()
{
	return m_nWidth;
}

int DX11DoubleBuffer::GetHeight()
{
	return m_nHeight;
}
