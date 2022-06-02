#include "CFoodieSharp.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CFoodieSharp::CFoodieSharp()
{
	m_pFBO = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_alpha = 0.0;
	m_EffectPart = SHARP_EFFECT;
}


CFoodieSharp::~CFoodieSharp()
{
    Release();
}

void CFoodieSharp::Release()
{
	SAFE_DELETE(m_pFBO);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
}

void * CFoodieSharp::Clone()
{
    CFoodieSharp* result = new CFoodieSharp();
    *result = *this;
    return result;
}

bool CFoodieSharp::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CFoodieSharp::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CFoodieSharp::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
    if (!childNode.isEmpty())
    {
        XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
        if (!nodeDrawable.isEmpty())
        {
            const char *szAlpha = nodeDrawable.getAttribute("Alpha");
            if(szAlpha!=NULL)
            {
                m_alpha = atof(szAlpha);
            }
			const char *szRadius = nodeDrawable.getAttribute("FilterRadis");
			if (szRadius != NULL)
			{
				m_Radius = atof(szRadius);
			}
			const char *szMaxSharp = nodeDrawable.getAttribute("MaxSharp");
			if (szMaxSharp != NULL)
			{
				m_MaxSharp = atof(szMaxSharp);
			}

        }
        return true;
    }
    return false;
}

bool CFoodieSharp::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	std::string  path = m_resourcePath + "/Shader/Beauty/FoodieGauss.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	path = m_resourcePath + "/Shader/Beauty/FoodieSharp.fx";
	m_pShaderSharp = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

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

void CFoodieSharp::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

	if (m_nWidth != nWidth || m_nHeight != nHeight)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		SAFE_DELETE(m_pFBO);
		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(m_nWidth, m_nHeight, false, NULL);
	}

    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	

	float pParam[4];
	pParam[0] =m_alpha;
	pParam[1] = 0;
	pParam[2] = m_Radius;
	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	pDoubleBuffer->SwapFBO();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

	pParam[2] = 0;
	pParam[1] = m_Radius * RenderParam.GetHeight() / RenderParam.GetWidth();
	m_pFBO->bind();
	m_pShader->useShader();
	pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);


	pParam[0] = 1 + m_alpha * m_MaxSharp;
	pDoubleBuffer->BindFBOA();
	m_pShaderSharp->useShader();

	pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	pSrcShaderView = m_pFBO->getTexture()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);



}


bool CFoodieSharp::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "FoodieSharp");
	nodeEffect.addAttribute("showname", m_showname.c_str());

	XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	
	char alpha[256];
	sprintf(alpha, "%.4f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	sprintf(alpha, "%.5f", m_Radius);
	nodeDrwable.addAttribute("FilterRadis", alpha);

	sprintf(alpha, "%.4f", m_MaxSharp);//0.5-5.0
	nodeDrwable.addAttribute("MaxSharp", alpha);

	return true;
}