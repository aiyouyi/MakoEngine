//
//  EffectModel.cpp
//  com.test.cc.ruf
//
//  Created by cc on 2017/6/14.
//  Copyright © 2017年 cc. All rights reserved.
//

#include "EffectModel.hpp"
#include <assert.h>
#include "Toolbox/DXUtils/DX11Context.h"
#include "Helper.h"

HeaderModel::HeaderModel() 
{
	m_arrPos = NULL;
	m_nVertex = 0;

	m_nFaces = 0;
	m_arrIndex = NULL;

	//渲染buffer
	m_headerVerticeBuffer = NULL;
	m_headerIndexBuffer = NULL;
}

void HeaderModel::destory()
{
	SAFERALEASE(m_headerVerticeBuffer);
	SAFERALEASE(m_headerIndexBuffer);
}

void HeaderModel::updateGpuBuffer()
{
	if (m_arrPos != NULL)
	{
		if (m_headerVerticeBuffer != NULL)
		{
			m_headerVerticeBuffer->Release();
			m_headerVerticeBuffer = NULL;
		}

		//创建顶点缓存
		D3D11_BUFFER_DESC verBufferDesc;
		memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		verBufferDesc.ByteWidth = sizeof(vec3) * m_nVertex;
		verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verBufferDesc.CPUAccessFlags = 0;
		verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA vertexInitData;
		memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexInitData.pSysMem = m_arrPos;
		DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_headerVerticeBuffer);
		m_arrPos = NULL;
	}
	

	//创建索引缓冲
	if (m_arrIndex != NULL)
	{
		if (m_headerIndexBuffer != NULL)
		{
			m_headerIndexBuffer->Release();
			m_headerIndexBuffer = NULL;
		}

		//创建索引buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		indexBufferDesc.ByteWidth = sizeof(WORD) * m_nFaces*3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA indexInitData;
		memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		indexInitData.pSysMem = m_arrIndex;
		DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &m_headerIndexBuffer);

		m_arrIndex = NULL;
	}
}


EffectModel::EffectModel()
{
	m_Layer = 0;

	m_arrVertices = NULL;
    m_nVertex = 0;
    
    m_nFaces = 0;
    m_arrIndex = NULL;
    
    m_drawable = NULL;

	m_bTransparent = true;
	m_fMixColor = vec4(1,1,1,1);

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
}

void EffectModel::destory()
{
	if (m_arrVertices != NULL)
	{
		delete[]m_arrVertices;
	}

	if (m_arrVertices != NULL)
	{
		delete[]m_arrIndex;
	}

	if (m_drawable != NULL)
	{
		delete m_drawable;
	}

	//渲染buffer
	if (m_rectVerticeBuffer != NULL)
	{
		m_rectVerticeBuffer->Release();
	}
	if (m_rectIndexBuffer != NULL)
	{
		m_rectIndexBuffer->Release();
	}
}


Effect2DRect::Effect2DRect()
{
	m_Layer = 1;

	m_x = 0;
	m_y = 0;
	m_width = 0;
	m_height = 0;

	m_nAlignType = EAPT_CC;

	m_nTargetWidth = 0;
	m_nTargetHeight = 0;

	m_bNeedUpdate = true;
	m_bAutoResize = true;
	m_fMixColor = vec4(1,1,1,1);

	m_arrVertices = new EffectVertex[4];
	memset(m_arrVertices, 0, sizeof(EffectVertex) * 4);

	m_arrVertices[0].vUV = vec2(0, 0);
	m_arrVertices[1].vUV = vec2(1, 0);
	m_arrVertices[2].vUV = vec2(0, 1);
	m_arrVertices[3].vUV = vec2(1, 1);

	m_arrIndex = new unsigned short[6];
	m_arrIndex[0] = 0;
	m_arrIndex[1] = 1;
	m_arrIndex[2] = 2;
	m_arrIndex[3] = 1;
	m_arrIndex[4] = 3;
	m_arrIndex[5] = 2;

	m_drawable = NULL;

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;


	m_pShader = NULL;
	m_pConstantBuffer = NULL;  //转换矩阵

	//采样状态
	m_pSamplerLinear = NULL;

	//混合以及buffer操作
	m_pBSEnable = NULL;
	m_pBSDisable = NULL;

	//深度操作
	m_pDepthStateDisable = NULL;

	m_bPrepare = false;
}
Effect2DRect::~Effect2DRect()
{
	SAFEDEL(m_pShader);

	SAFERALEASE(m_pConstantBuffer);
	//采样状态
	SAFERALEASE(m_pSamplerLinear);

	//混合以及buffer操作
	SAFERALEASE(m_pBSEnable);
	SAFERALEASE(m_pBSDisable);

	//深度操作
	SAFERALEASE(m_pDepthStateDisable);
}
void Effect2DRect::updateRenderInfo(int nTargetWidth, int nTargetHeight)
{
	assert(nTargetWidth>0);
	assert(nTargetHeight>0);

	if (m_nTargetWidth != nTargetWidth || m_nTargetHeight != nTargetHeight || m_bNeedUpdate)
	{
		m_bNeedUpdate = true;
		m_nTargetWidth = nTargetWidth;
		m_nTargetHeight = nTargetHeight;
	}

	if (m_bNeedUpdate)
	{
		m_bNeedUpdate = false;

		int w = m_width;
		int h = m_height;

		float fStepY = 2.0f / DESIGN_BASESIZE;
		float fStepX = fStepY*nTargetHeight / nTargetWidth;

		if (nTargetHeight > nTargetWidth && m_bAutoResize)
		{
			float fStepX = 2.0f / DESIGN_BASESIZE;
			float fStepY = fStepX*nTargetWidth / nTargetHeight;
		}

		float fLBX = 0;
		float fLBY = 0;
		float fLogoW = w*fStepX;
		float fLogoH = h*fStepY;
		/*
		EAPT_LT = 0,  //x,y表示距离左上角距离
		EAPT_LB = 1,  //x,y表示距离左下角距离
		EAPT_RT = 2,  //x,y表示距离右上角距离
		EAPT_RB = 3,   //x,y表示距离右下角距离
		EAPT_CT=4,     //x,y表示距离上边中心距离
		EAPT_CB=5,     //x,y表示距离底边中心距离
		EAPT_LC=6,     //x,y表示左边中心距离
		EAPT_RC=7,     //x,y表示右边中心距离
		EAPT_CC
		*/
		switch (m_nAlignType)
		{
		case EAPT_LT:
		{
			fLBX = -1.0f + m_x*fStepX;
			fLBY = -1.0f + m_y*fStepY;
			break;
		}
		case EAPT_LB:
		{
			fLBX = -1.0f + m_x*fStepX;
			fLBY = 1.0f - m_y*fStepY - fLogoH;
			break;
		}
		case EAPT_RT:
		{
			fLBX = 1.0f - m_x*fStepX - fLogoW;
			fLBY = -1.0f + m_y*fStepY;
			break;
		}
		case EAPT_RB:
		{
			fLBX = 1.0f - m_x*fStepX - fLogoW;
			fLBY = 1.0f - m_y*fStepY - fLogoH;
			break;
		}

		case EAPT_CT:
		{
			fLBX = m_x*fStepX - fLogoW*0.5f;
			fLBY = -1.0f + m_y*fStepY;
			break;
		}
		case EAPT_CB:
		{
			fLBX = m_x*fStepX - fLogoW*0.5f;
			fLBY = 1.0f - m_y*fStepY - fLogoH;
			break;
		}


		case EAPT_LC:
		{
			fLBX = -1.0f + m_x*fStepX;
			fLBY = m_y*fStepY - fLogoH*0.5f;
			break;
		}
		case EAPT_RC:
		{
			fLBX = 1.0f - m_x*fStepX - fLogoW;
			fLBY = m_y*fStepY - fLogoH*0.5f;
			break;
		}
		case EAPT_CC:
		{
			fLBX = m_x*fStepX - fLogoW*0.5f;
			fLBY = m_y*fStepY - fLogoH*0.5f;
			break;
		}
		}

		m_arrVertices[0].vPos = vec3(fLBX, fLBY, 0);
		m_arrVertices[1].vPos = vec3(fLBX + fLogoW, fLBY, 0);
		m_arrVertices[2].vPos = vec3(fLBX, fLBY + fLogoH, 0);
		m_arrVertices[3].vPos = vec3(fLBX + fLogoW, fLBY + fLogoH, 0);
		for (int i = 0; i < 4; ++i)
		{
			m_arrVertices[i].vPos.y = 0.0f - m_arrVertices[i].vPos.y;
		}

		if (m_rectVerticeBuffer != NULL)
		{
			m_rectVerticeBuffer->Release();
		}
		//创建顶点缓存
		D3D11_BUFFER_DESC verBufferDesc;
		memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		verBufferDesc.ByteWidth = sizeof(EffectVertex) * 4;
		verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verBufferDesc.CPUAccessFlags = 0;
		verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA vertexInitData;
		memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexInitData.pSysMem = m_arrVertices;
		DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_rectVerticeBuffer);

		//创建索引缓冲
		if (m_rectIndexBuffer == NULL)
		{
			//创建索引buffer
			D3D11_BUFFER_DESC indexBufferDesc;
			memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
			indexBufferDesc.ByteWidth = sizeof(WORD) * 6;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			D3D11_SUBRESOURCE_DATA indexInitData;
			memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
			indexInitData.pSysMem = m_arrIndex;
			DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &m_rectIndexBuffer);
		}
	}
}

void Effect2DRect::updateRenderInfo2(int nTargetWidth, int nTargetHeight)
{

	assert(nTargetWidth > 0);
	assert(nTargetHeight > 0);

	if (m_nTargetWidth != nTargetWidth || m_nTargetHeight != nTargetHeight || m_bNeedUpdate)
	{
		m_bNeedUpdate = true;
		m_nTargetWidth = nTargetWidth;
		m_nTargetHeight = nTargetHeight;
	}

	if (m_bNeedUpdate)
	{
		m_bNeedUpdate = false;

		int w = m_width;
		int h = m_height;

		float fStepY = 720.f / nTargetHeight;

		float fStepX = 1280.f / nTargetWidth;

		if (fStepX>fStepY)
		{
			fStepX /= fStepY;
			fStepY = 1.0;
		}
		else
		{
			fStepY /= fStepX;
			fStepX = 1.0;
		}



		float fLBX = 0;
		float fLBY = 0;
		float fLogoW = m_width * fStepX / 1280;
		float fLogoH = m_height*fStepY/720;
		float x = m_x * 1.f / 1280;
		float y = m_y * 1.f / 720;
		switch (m_nAlignType)
		{
		case EAPT_LT:
		{
			fLBX =x * fStepX;
			fLBY = y * fStepY;
			break;
		}
		case EAPT_LB:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepY>1.0)
			{
				float offest = 1.0 - 1.0/fStepY;
				fLBY = (y - offest)*fStepY;
			}
// 			else
// 			{
// 				fLBY = y * nTargetHeight / 720;;
// 			}
			break;
		}
		case EAPT_RT:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepX>1.0)
			{
				float offest = 1.0 - 1.0 / fStepX;
				fLBX = (x - offest)*fStepX;
			}
			break;
		}
		case EAPT_RB:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepX > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepX;
				fLBX = (x - offest)*fStepX;
			}
			if (fStepY > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepY;
				fLBY = (y - offest)*fStepY;
			}
			break;
		}

		case EAPT_CT:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepX > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepX;
				fLBX = (x - offest*0.5)*fStepX;
			}
			break;
		}
		case EAPT_CB:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepX > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepX;
				fLBX = (x - offest * 0.5)*fStepX;
			}
			if (fStepY > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepY;
				fLBY = (y - offest)*fStepY;
			}
			break;
		}


		case EAPT_LC:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepY > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepY;
				fLBY = (y - offest*0.5)*fStepY;
			}
			break;
		}
		case EAPT_RC:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepX > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepX;
				fLBX = (x - offest)*fStepX;
			}
			if (fStepY > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepY;
				fLBY = (y - offest * 0.5)*fStepY;
			}
			break;
		}
		case EAPT_CC:
		{
			fLBX = x * fStepX;
			fLBY = y * fStepY;
			if (fStepX > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepX;
				fLBX = (x - offest*0.5)*fStepX;
			}
			if (fStepY > 1.0)
			{
				float offest = 1.0 - 1.0 / fStepY;
				fLBY = (y - offest * 0.5)*fStepY;
			}
			break;
		}

		case EAPT_SCALE:
		{
			fLBX = x ;
			fLBY = y ;
			fLogoW = m_width*1.f / 1280;
			fLogoH = m_height*1.f / 720;
		}
		}

		m_arrVertices[0].vPos = vec3(fLBX, fLBY, 0);
		m_arrVertices[1].vPos = vec3(fLBX + fLogoW, fLBY, 0);
		m_arrVertices[2].vPos = vec3(fLBX, fLBY + fLogoH, 0);
		m_arrVertices[3].vPos = vec3(fLBX + fLogoW, fLBY + fLogoH, 0);

		for (int i = 0; i < 4; ++i)
		{
			m_arrVertices[i].vPos = m_arrVertices[i].vPos*2.0 - vec3(1.0,1.0,1.0);
		}
// 		for (int i = 0; i < 4; ++i)
// 		{
// 			m_arrVertices[i].vPos.y = 0.0f - m_arrVertices[i].vPos.y;
// 		}
		if (m_rectVerticeBuffer != NULL)
		{
			m_rectVerticeBuffer->Release();
		}
		//创建顶点缓存
		D3D11_BUFFER_DESC verBufferDesc;
		memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
		verBufferDesc.ByteWidth = sizeof(EffectVertex) * 4;
		verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verBufferDesc.CPUAccessFlags = 0;
		verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA vertexInitData;
		memset(&vertexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexInitData.pSysMem = m_arrVertices;
		DevicePtr->CreateBuffer(&verBufferDesc, &vertexInitData, &m_rectVerticeBuffer);

		//创建索引缓冲
		if (m_rectIndexBuffer == NULL)
		{
			//创建索引buffer
			D3D11_BUFFER_DESC indexBufferDesc;
			memset(&indexBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
			indexBufferDesc.ByteWidth = sizeof(WORD) * 6;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			D3D11_SUBRESOURCE_DATA indexInitData;
			memset(&indexInitData, 0, sizeof(D3D11_SUBRESOURCE_DATA));
			indexInitData.pSysMem = m_arrIndex;
			DevicePtr->CreateBuffer(&indexBufferDesc, &indexInitData, &m_rectIndexBuffer);
		}
	}

}

void Effect2DRect::setRect(int x, int y, int width, int height, int AlignType, bool bAutoResize)
{
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;

	if (AlignType < EAPT_MAX)
	{
		m_nAlignType = AlignType;
	}
	else
	{
		m_nAlignType = EAPT_CC;
	}

	m_bAutoResize = bAutoResize;

	invalidate();
}

void Effect2DRect::setLayer(int layer)
{
	m_Layer = layer;
}

void Effect2DRect::invalidate()
{
	m_bNeedUpdate = true;
}

void Effect2DRect::destory()
{
	if (m_arrVertices != NULL)
	{
		delete[]m_arrVertices;
		m_arrVertices = NULL;
	}

	if (m_arrIndex != NULL)
	{
		delete[]m_arrIndex;
		m_arrIndex = NULL;
	}

	if (m_drawable != NULL)
	{
		delete m_drawable;
		m_drawable = NULL;
	}

	if (m_rectVerticeBuffer != NULL)
	{
		m_rectVerticeBuffer->Release();
		m_rectVerticeBuffer = NULL;
	}

	if (m_rectIndexBuffer != NULL)
	{
		m_rectIndexBuffer->Release();
		m_rectIndexBuffer = NULL;
	}
}

void Effect2DRect::render(int width, int height, float during)
{
	prepare();

	//绘制2D贴图效果
	m_pShader->useShader();
	EffectConstantBuffer matWVP;
	matWVP.mWVP = XMMatrixIdentity();
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBSEnable, blendFactor, 0xffffffff);
	DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateDisable, 0);

	updateRenderInfo(width, height);

	unsigned int nStride = sizeof(EffectVertex);
	unsigned int nOffset = 0;

	//设置矩阵变换
	matWVP.mColor = XMVectorSet(m_fMixColor.x, m_fMixColor.y, m_fMixColor.z, m_fMixColor.w);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &matWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//设置纹理以及纹理采样
	ID3D11ShaderResourceView *pMyShaderResourceView = m_drawable->GetSRV(during);
	if (pMyShaderResourceView != NULL)
	{
		DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		//设置顶点数据
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	}
}

extern const char *s_szStickerShader;
bool Effect2DRect::prepare()
{
	if (m_bPrepare)return true;
	m_bPrepare = true;

	//创建纹理采样
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = DevicePtr->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
	if (FAILED(hr))return false;

	//混合状态
	if (m_pBSEnable == NULL)
	{
		m_pBSEnable = ContextInst->fetchBlendState(true, false, true);
	}
	if (m_pBSDisable == NULL)
	{
		m_pBSDisable = ContextInst->fetchBlendState(false, false, true);
	}

	if (m_pDepthStateDisable == NULL)
	{
		m_pDepthStateDisable = ContextInst->fetchDepthStencilState(false, false);
	}

	//渲染背景的shader
	//渲染模型信息
	{
		//创建shader
		m_pShader = new DX11Shader();
		m_pShader->initShaderWithString(s_szStickerShader);

		//创建constbuffer 参数
		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(EffectConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
		if (FAILED(hr))return false;
	}
	return true;
}