#include "CFaceLevel.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceLevel::CFaceLevel()
{
	m_alpha = 0.f;
	
	m_nWidth = 0;
	m_nHeight = 0;

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;

	m_EffectPart = FACE_LEVELS;
}


CFaceLevel::~CFaceLevel()
{
	Release();
}

void CFaceLevel::Release()
{
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
}

void * CFaceLevel::Clone()
{
	CFaceLevel* result = new CFaceLevel();
	*result = *this;
	return result;
}

bool CFaceLevel::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szAlpha = nodeDrawable.getAttribute("Alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}

		}
		return true;
	}
	return false;
}

bool CFaceLevel::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	std::string  path = m_resourcePath + "/Shader/Beauty/FaceLevel.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	static const float g_TextureCoordinate[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
	if (m_rectIndexBuffer == NULL)
	{
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};
		m_rectIndexBuffer = DXUtils::CreateIndexBuffer(index, 2);
	}
	if (m_rectVerticeBuffer == NULL)
	{
		m_rectVerticeBuffer = DXUtils::CreateVertexBuffer((float*)g_TextureCoordinate, 4, 2);
	}

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	return true;
}

void CFaceLevel::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	float radius = 0.002;

	float pParam[4];
	pParam[0] = m_alpha / 255.f;
	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureA());
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	pDoubleBuffer->SwapFBO();

}
