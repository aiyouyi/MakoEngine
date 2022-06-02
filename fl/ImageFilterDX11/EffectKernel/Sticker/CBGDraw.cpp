#include "CBGDraw.h"
#include "common.h"
#include <xnamath.h>
#include<BaseDefine/Vectors.h>
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/Render/DynamicRHI.h"

struct RectConstantMask
{
	XMMATRIX mWVP; //混合矩阵
	XMFLOAT4 mClip;//背景裁剪区域
};

CBGDraw::CBGDraw()
{
	m_nEffectDuring = -1;
	m_nAniLoopTime = -1;
	m_nStartTime = -1;
	m_nLoopStartTime = -1;

	m_2DBGAnimation = NULL;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBufferMask = NULL;
}


CBGDraw::~CBGDraw()
{
	
	Release();
}

void * CBGDraw::Clone()
{
	CBGDraw* result = new CBGDraw();
	*result = *this;
	return result;
}

bool CBGDraw::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty()) {
			const char* szDrawableName = nodeDrawable.getAttribute("name");
			AnimationDrawable* drawable = new AnimationDrawable();

			const char* szOffset = nodeDrawable.getAttribute("offset");
			long offset = atol(szOffset);
			drawable->setOffset(offset);

			const char* szLoopMode = nodeDrawable.getAttribute("loopMode");
			if (szLoopMode != NULL && strcmp(szLoopMode, "oneShot") == 0)
				drawable->setLoopMode(ELM_ONESHOT);
			else
				drawable->setLoopMode(ELM_REPEAT);

			const char* szGenMipmap = nodeDrawable.getAttribute("genMipmaps");
			bool bGenMipmap = false;
			if (szGenMipmap != NULL && (strcmp(szGenMipmap, "yes") || strcmp(szGenMipmap, "YES")))
				bGenMipmap = true;

			//解析动画信息
			int j = -1;
			XMLNode nodeItem = nodeDrawable.getChildNode("item", ++j);
			while (!nodeItem.isEmpty()) {
				char szFullFile[256];
				const char* szImagePath = nodeItem.getAttribute("image");
				sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);

				std::shared_ptr<CC3DTextureRHI> TexRHI = GetDynamicRHI()->FetchTexture(szFullFile,false);
				if (!TexRHI)
				{
					TexRHI = GetDynamicRHI()->CreateTextureFromZip(hZip, szImagePath, bGenMipmap);
					GetDynamicRHI()->RecoredTexture(szFullFile, TexRHI);
				}

				const char* szDuring = nodeItem.getAttribute("duration");
				long during = atol(szDuring);

				drawable->appandTex(during, TexRHI);

				nodeItem = nodeDrawable.getChildNode("item", ++j);
			}
			m_2DBGAnimation = drawable;
		}


	}
	return false;
}

bool CBGDraw::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2},
	};

	std::string path = m_resourcePath + "/Shader/BGDraw.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	float vertices[] =
	{
		-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,0.0f, 1.0f,
		1.0f, -1.0f, 0.0f,1.0f, 1.0f,
		1, 1.0f, 0.0f ,1.0f, 0.0f
	};

	m_rectVerticeBuffer = DXUtils::CreateVertexBuffer(vertices, 4, 5);

	unsigned short index[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	m_rectIndexBuffer = DXUtils::CreateIndexBuffer(index, 2);

	m_pConstantBufferMask = DXUtils::CreateConstantBuffer(sizeof(RectConstantMask));
	return true;
}

void CBGDraw::Render(BaseRenderParam & RenderParam)
{
	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();

	if (m_nStartTime < 0)
	{
		m_nLoopStartTime = m_nStartTime = ccCurrentTime();
	}

	__int64 currentTime = ccCurrentTime();
	long runTime = long(currentTime - m_nStartTime);
	long loopTime = long(currentTime - m_nLoopStartTime);
	long during = runTime;
	if (m_nAniLoopTime > 0)
	{
		if (m_nEffectDuring < 0)
		{
			m_nEffectDuring = m_nAniLoopTime;
		}

		during = during % m_nAniLoopTime;
	}

	float arrClip[] = { 0,0,1,1 };

	ID3D11ShaderResourceView *pBGRV = m_2DBGAnimation->GetSRV(during);
	int dw, dh;
	m_2DBGAnimation->getSize(dw, dh);

	if (dw > 0 && dh > 0)
	{
		float fImgRatio = 1.0f*dw / dh;
		float fRTRatio = 1.0f * width / height;
		if (fImgRatio > fRTRatio)
		{
			float fClipX = fRTRatio / fImgRatio;
			fClipX = 1.0f - fClipX;
			arrClip[0] = fClipX * 0.5f;
			arrClip[2] = 1.0f - fClipX * 0.5f;
		}
		else
		{
			fImgRatio = 1.0f*dh / dw;
			fRTRatio = 1.0f*height / width;
			float fClipY = fRTRatio / fImgRatio;
			fClipY = 1.0f - fClipY;
			arrClip[1] = fClipY * 0.5f;
			arrClip[3] = 1.0f - fClipY * 0.5f;
		}
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();

	m_pShader->useShader();
	auto pMaterialView = pBGRV;
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

	RectConstantMask mWVP;
	mWVP.mWVP = XMMatrixIdentity();
	mWVP.mClip = XMFLOAT4(arrClip[0], arrClip[1], arrClip[2], arrClip[3]);
	DeviceContextPtr->UpdateSubresource(m_pConstantBufferMask, 0, NULL, &mWVP, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBufferMask);


	//设置顶点数据
	unsigned int nStride = 5 * sizeof(float);
	unsigned int nOffset = 0;
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

void CBGDraw::Release()
{
	SAFE_DELETE(m_2DBGAnimation);
	SAFE_RELEASE_BUFFER(m_pConstantBufferMask);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
}
