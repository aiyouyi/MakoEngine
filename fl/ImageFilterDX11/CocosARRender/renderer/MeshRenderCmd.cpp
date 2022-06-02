#include "MeshRenderCmd.h"
#include "Toolbox/Helper.h"
#include "CShaderCache.h"
#include <iostream>
using namespace std;
NS_CC_BEGIN
MeshRenderCmd::MeshRenderCmd()
{
	m_bNeedUpdate = false;
	m_bTransparent = false;
	m_bLighting = false;
	m_pMesh = NULL;

	//渲染相关接口
	m_pShader = NULL;
	m_pConstantBuffer = NULL;

	//采样状态
	m_pSamplerLinear = NULL;

	//混合以及buffer操作
	m_pBlendState = NULL;

	//深度操作
	m_pDepthState = NULL;
	m_pDepthStateRestore = NULL;
}

MeshRenderCmd::~MeshRenderCmd()
{
	destory();
}

bool MeshRenderCmd::init(Mesh *pMesh, Mat4 matTrans, bool bTransparent, bool bLighting,bool bNPR)
{
	destory();

	if (m_pMesh != pMesh)
	{
		m_pMesh = pMesh;

		if (m_pMesh != NULL)
		{
			m_pMesh->retain();
		}
	}

	m_matTrans = matTrans;
	m_matTransInit = matTrans;
	m_bTransparent = bTransparent;
	m_bLighting = bLighting;
	m_bNPR = bNPR;

	m_bNeedUpdate = true;
	return true;
}

const Mat4 MeshRenderCmd::getInitWTransform()
{
	return m_matTransInit;
}

bool MeshRenderCmd::isTransparent()
{
	return m_bTransparent;
}

void MeshRenderCmd::updateTransform(const Mat4 &matTransform)
{
	m_matTrans = matTransform;
}

float MeshRenderCmd::getZOrder()
{
	if (m_pMesh == NULL || m_pMesh->getMeshIndexData() == NULL)
	{
		return 0.0f;
	}

	AABB aabb = m_pMesh->getMeshIndexData()->getAABB();
	Vec3 vCenter = aabb.getCenter();

	m_matTrans.transformPoint(&vCenter);
	return vCenter.z;
}

void MeshRenderCmd::render(const Mat4 &matView, const Mat4 &matProj, vector<BaseLight> &vLights, NPRInfo *npr)
{
	if (m_pMesh == NULL || m_pMesh->GetTexture() == NULL)return;

	prepare();


	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	MeshIndexData *meshIndexData = m_pMesh->getMeshIndexData(); 

	DeviceContextPtr->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);

	//控制混合状态
// 	D3D11_BLEND_DESC blend_desc;
// 	memset(&blend_desc, 0, sizeof(blend_desc));
// 	blend_desc.AlphaToCoverageEnable = false;
// 	blend_desc.IndependentBlendEnable = false;
// 	blend_desc.RenderTarget[0].BlendEnable = true;
// 	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
// 	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
// 	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
// 	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
// 	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
// 	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
// 	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
// 
// 	ID3D11BlendState *pBlendState = NULL;
// 	DevicePtr->CreateBlendState(&blend_desc, &pBlendState);
// 	float blendFactor[] = { 0.f,0.f,0.f,0.f };
// 	DeviceContextPtr->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);


	DeviceContextPtr->OMSetDepthStencilState(m_pDepthState, 0);

	SkinLightingNPRCB SCB_NPR;
	SkinLightingCB SCB;

	//渲染数据..
	if(m_pShader != NULL)
	{
		m_pShader->useShader();
		if (m_pMesh->getSkin() == NULL)
		{
			BaseLightingCB cb;
			cb.matWorld = m_matTrans;
			Mat4 matVP = matProj*matView;
			cb.matVP = matVP;

			memset(&(cb.arrLight), 0, sizeof(BaseLightInfo) * 4);
			for (int i = 0; i < vLights.size() && i < 4; ++i)
			{
				cb.arrLight[i].vAmbient = vLights[i].m_vAmbient;
				cb.arrLight[i].vDiffuse = vLights[i].m_vDiffuse;
				cb.arrLight[i].vSpecular = vLights[i].m_vSpecular;
				cb.arrLight[i].vOrient = vLights[i].m_vOrientation;
			}

			//设置矩阵变换
			DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);
			DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		}
		else if(m_bNPR)
		{
			Vec4 *vMatrixPalette = m_pMesh->getSkin()->getMatrixPalette();
			int nPalette = m_pMesh->getSkin()->getMatrixPaletteSize();

			
			SCB_NPR.matWorld = m_matTrans;
			Vec3 scale; Quaternion rotation; Vec3 translation;
			SCB_NPR.matWorld.decompose(&scale, &rotation, &translation);
			Mat4::createRotation(rotation, &(SCB_NPR.matWorldR));
			Mat4 matVP = matProj*matView;
			SCB_NPR.matVP = matVP;
			memcpy(SCB_NPR.matPalette, vMatrixPalette, sizeof(Vec4) * nPalette);
			memset(&(SCB_NPR.arrLight), 0, sizeof(BaseLightInfo) * 4);
			for (int i = 0; i < vLights.size() && i < 4; ++i)
			{
				SCB_NPR.arrLight[i].vAmbient = vLights[i].m_vAmbient;
				SCB_NPR.arrLight[i].vDiffuse = vLights[i].m_vDiffuse;
				SCB_NPR.arrLight[i].vSpecular = vLights[i].m_vSpecular;
				SCB_NPR.arrLight[i].vOrient = vLights[i].m_vOrientation;
			}
			SCB_NPR.u_NPRInfo.x = npr->m_fLineWidth;
			SCB_NPR.u_NPRInfo.y = npr->m_vZOffest.x;
			SCB_NPR.u_NPRInfo.z = npr->m_vZOffest.y;
			SCB_NPR.u_NPRInfo.w = npr->m_vZOffest.z;
			SCB_NPR.u_NPRLineColor = npr->m_vLineColor;

			//设置矩阵变换
			DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &SCB_NPR, 0, 0);
			DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		}
		else
		{
			Vec4 *vMatrixPalette = m_pMesh->getSkin()->getMatrixPalette();
			int nPalette = m_pMesh->getSkin()->getMatrixPaletteSize();


			SCB.matWorld = m_matTrans;
			Vec3 scale; Quaternion rotation; Vec3 translation;
			SCB.matWorld.decompose(&scale, &rotation, &translation);
			Mat4::createRotation(rotation, &(SCB.matWorldR));
			Mat4 matVP = matProj * matView;
			SCB.matVP = matVP;
			memcpy(SCB.matPalette, vMatrixPalette, sizeof(Vec4) * nPalette);
			memset(&(SCB.arrLight), 0, sizeof(BaseLightInfo) * 4);
			for (int i = 0; i < vLights.size() && i < 4; ++i)
			{
				SCB.arrLight[i].vAmbient = vLights[i].m_vAmbient;
				SCB.arrLight[i].vDiffuse = vLights[i].m_vDiffuse;
				SCB.arrLight[i].vSpecular = vLights[i].m_vSpecular;
				SCB.arrLight[i].vOrient = vLights[i].m_vOrientation;
			}
			//设置矩阵变换
			DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &SCB, 0, 0);
			DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		}
		//设置纹理以及纹理采样
		ID3D11ShaderResourceView *pMyShaderResourceView = m_pMesh->GetTexture()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(0, 1, &pMyShaderResourceView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

		//设置顶点数据
		unsigned int nStride = meshIndexData->getVertexBuffer()->getSizePerVertex();
		unsigned int nOffset = 0;
		ID3D11Buffer *vbo = meshIndexData->getVertexBuffer()->getVBO();

		DeviceContextPtr->IASetVertexBuffers(0, 1, &(vbo), &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(meshIndexData->getIndexBuffer()->getVBO(), DXGI_FORMAT_R16_UINT, 0);

		DeviceContextPtr->DrawIndexed(meshIndexData->getIndexBuffer()->getIndexNumber(), 0, 0);


		if (m_pMesh->getSkin() != NULL && m_bNPR)
		{
			SCB_NPR.u_NPRInfo.x = 0.0;
			//设置矩阵变换
			DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, &SCB_NPR, 0, 0);
			DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
			DeviceContextPtr->DrawIndexed(meshIndexData->getIndexBuffer()->getIndexNumber(), 0, 0);
		}
	}
	DeviceContextPtr->OMSetDepthStencilState(m_pDepthStateRestore, 0);
	

// 	DeviceContextPtr->OMSetBlendState(0, blendFactor, 0xffffffff);
// 	pBlendState->Release();
}

void MeshRenderCmd::prepare()
{
	if (m_bNeedUpdate)
	{
		m_bNeedUpdate = false;

		MeshVertexAttrib attribs[16];
		int nCount = m_pMesh->getMeshVertexAttribCount();
		for (int i = 0; i < nCount; ++i)
		{
			attribs[i] = m_pMesh->getMeshVertexAttribute(i);
		}

		m_pShader = CCShaderCache::shareInst()->fetchShader(attribs, nCount, m_bLighting,m_bNPR);

		D3D11_BUFFER_DESC bd;
		memset(&bd, 0, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (m_pMesh->getSkin() != NULL)
		{
			if (m_bLighting)
			{
				if (m_bNPR)
				{
					bd.ByteWidth = sizeof(SkinLightingNPRCB);
				}
				else
				{
					bd.ByteWidth = sizeof(SkinLightingCB);
				}
			}
			else
			{
				bd.ByteWidth = sizeof(SkinConstantBuffer);
			}
		}
		else
		{
			if (m_bLighting)
			{
				bd.ByteWidth = sizeof(BaseLightingCB);
			}
			else
			{
				bd.ByteWidth = sizeof(BaseConstantBuffer);
			}
		}
		HRESULT hr = DevicePtr->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
		if (FAILED(hr))
		{
			return;
		}

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
		hr = DevicePtr->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
		if (FAILED(hr))return;

		//混合状态
		if (m_bTransparent)
		{
			m_pBlendState = ContextInst->fetchBlendState(true, false, true);
			m_pDepthState = ContextInst->fetchDepthStencilState(true, false);
		}
		else
		{
			m_pBlendState = ContextInst->fetchBlendState(false, false, true);
			m_pDepthState = ContextInst->fetchDepthStencilState(true, true);
		}

		if (m_pDepthStateRestore == NULL)
		{
			m_pDepthStateRestore = ContextInst->fetchDepthStencilState(false, false);
		}
	}
}

void MeshRenderCmd::destory()
{
	if (m_pMesh != NULL)
	{
		m_pMesh->release();
		m_pMesh = NULL;
	}

	SAFERALEASE(m_pConstantBuffer);

	SAFERALEASE(m_pSamplerLinear);

	//混合以及buffer操作
	SAFERALEASE(m_pBlendState);

	//深度操作
	SAFERALEASE(m_pDepthState);
	SAFERALEASE(m_pDepthStateRestore);
}

NS_CC_END