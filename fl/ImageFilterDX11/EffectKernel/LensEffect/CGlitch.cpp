#include "CGlitch.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

CGlitch::CGlitch()
{
    //m_material = 0;
    m_alpha = 1.0;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_EffectPart = GLITCH_EFFECT;
}


CGlitch::~CGlitch()
{
    Release();
}

void CGlitch::Release()
{
	//SAFE_DELETE(m_material);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
}

void * CGlitch::Clone()
{
	CGlitch* result = new CGlitch();
    *result = *this;
    return result;
}

bool CGlitch::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CGlitch::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CGlitch::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	std::string  path = m_resourcePath + "/Shader/LensEffect/Giltch.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);
	SAFE_RELEASE_BUFFER(m_pBlendStateNormal);
	m_pBlendStateNormal = DXUtils::CloseBlendState();

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
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 12);
	}

	return true;
}

void CGlitch::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}
	long runTime = GetRunTime();

	int loop = (runTime - m_StartTime) / m_Alltime;
	if (loop >= m_Loop && m_Loop != 0)
	{
		return;
	}

	runTime = runTime % (int)m_Alltime;
	if (runTime >m_DuringTime)
	{
		return;
	}

	float alpha =1.0- (runTime % (int)m_DuringTime)*1.f / m_DuringTime;

	float pParam[12];
	pParam[0] = 0.02*alpha*RenderParam.GetHeight() / RenderParam.GetWidth();
	pParam[1] =  0.5*alpha + 0.5;

	pParam[2] = 0.025*RenderParam.GetHeight() / RenderParam.GetWidth();
	pParam[3] = 0.08;

	pParam[4] = m_alpha;

	pParam[5] = 1.0;
	pParam[6] = 0.0;
	pParam[7] =0;
	pParam[8] = 0;



// 	pParam[0] = m_alpha * 0.3 *alpha;
// 	pParam[1] =1/( (1 - alpha) * 5 + 1);
	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SwapFBO();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

bool CGlitch::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("param", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			szAlpha = nodeDrawable.getAttribute("Alltime");
			if (szAlpha != NULL)
			{
				m_Alltime = atof(szAlpha);
			}
			szAlpha = nodeDrawable.getAttribute("Onetime");
			if (szAlpha != NULL)
			{
				m_DuringTime = atof(szAlpha);
			}
			const char * szLoop = nodeDrawable.getAttribute("Loopnum");
			if (szLoop != NULL)
			{
				m_Loop = atoi(szLoop);
			}
		}
		return true;
	}
	return false;
}

bool CGlitch::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "Glitch");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	XMLNode nodeDrwable = nodeEffect.addChild("param");
	
	char alpha[256];
	sprintf(alpha, "%.2f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	sprintf(alpha, "%.2f", m_Alltime);
	nodeDrwable.addAttribute("Alltime", alpha);

	sprintf(alpha, "%.2f", m_DuringTime);
	nodeDrwable.addAttribute("Onetime", alpha);

	sprintf(alpha, "%d", m_Loop);
	nodeDrwable.addAttribute("Loopnum", alpha);

	return true;
}

void CGlitch::SetEffectCount(int count)
{
	m_Loop = count;
	m_StartTime = GetRunTime();
}