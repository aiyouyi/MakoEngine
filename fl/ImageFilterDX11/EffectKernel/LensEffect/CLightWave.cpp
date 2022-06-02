#include "CLightWave.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CLightWave::CLightWave()
{
	m_pShaderSobel = NULL;
	m_pShaderBlur = NULL;
    m_alpha = 0.4;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_pBlendState = NULL;
	m_pFBO = NULL;
	m_EffectPart = LIGHTWAVE_EFFECT;
}


CLightWave::~CLightWave()
{
    Release();
}

void CLightWave::Release()
{
	SAFE_DELETE(m_pFBO);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);
	SAFE_RELEASE_BUFFER(m_pBlendState);
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	
}


bool CLightWave::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CLightWave::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CLightWave::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};

	std::string path = m_resourcePath + "/Shader/LensEffect/lightWaveFilter.fx";
	m_pShaderBlur = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	path = m_resourcePath + "/Shader/LensEffect/sobelFilter.fx";
	m_pShaderSobel = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	path = m_resourcePath + "/Shader/LensEffect/LightWave.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);
	//SAFE_RELEASE_BUFFER(m_pBlendStateNormal);
	//m_pBlendStateNormal = DXUtils::CloseBlendState();

	m_pBlendState = DXUtils::CreateBlendState(D3D11_BLEND_ONE,D3D11_BLEND_INV_SRC_ALPHA);
	m_pSamplerLinear = DXUtils::SetSampler(D3D11_TEXTURE_ADDRESS_MIRROR);

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

void CLightWave::Render(BaseRenderParam &RenderParam)
{
	auto BodyTexture = RenderParam.GetBodyMaskTexture();
	if (BodyTexture == NULL)
	{
		return;
	}

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

	long runTime1 = runTime % (int)m_Alltime;
	int light_flag = 1.0;
	if ((runTime1 > 4*m_DuringTime) ||  (runTime1 >m_DuringTime && runTime1 < 2 * m_DuringTime))
	{
		light_flag = 0.0;
		//return;
	}
	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

	float alpha = (runTime1 % (int)m_DuringTime)*1.f / m_DuringTime;
	float pParam[4];

	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	if (m_nWidth != nWidth || m_nHeight != nHeight)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		SAFE_DELETE(m_pFBO);
		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(m_nWidth , m_nHeight, false, NULL);//*m_ScaleHW
	}

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	
	//get blur filter result in FBO
	pDoubleBuffer->BindFBOB();
	m_pShaderBlur->useShader();
	pParam[0] = 0.0;
	pParam[1] = 1.f / nHeight*0.4;
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);


	m_pFBO->bind();
	m_pShaderBlur->useShader();
	pParam[0] = 1.f / nWidth*0.4;
	pParam[1] = 0.0;
	pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	
	//Second get guide filter and then get sobel result
	pDoubleBuffer->BindFBOB();
	m_pShaderSobel->useShader();
	pParam[0] = 1.f / nWidth * m_alpha;
	pParam[1] = 1.f / nHeight * m_alpha;
	pParam[2] = alpha * 0.2 + 0.5;

	pSrcShaderView = m_pFBO->getTexture()->getTexShaderView();
	auto pMaskView = BodyTexture->getTexShaderView();

	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

	//get light wave result
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendState, blendFactor, 0xffffffff);
	//****param[0-1] uv offset
	if (alpha <= 0.5)
	{
		pParam[0] = alpha * 0.4;
		pParam[1] = alpha * 0.15;
	}
	else
	{
		pParam[0] = (0.5 - alpha) * 0.2;
		pParam[1] = (alpha-0.5) * 0.15;
	}
	pParam[2] = light_flag;
	pParam[3] = int((runTime % 80000) / int(m_DuringTime))*m_ColorSpeed;

	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();

	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, float(runTime1));
	auto pMaterialView = img->tex->getTexShaderView();
	pMaskView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();

	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);

}

bool CLightWave::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
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

			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					m_anim_id = ResourceManager::Instance().loadAnim(anim_info);

					//m_drawable = new BitmapDrawable(pTex);
				}
				else
				{
					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
					//m_drawable = new BitmapDrawable(pTex);
				}

			}
		}
		return true;
	}
	return false;
}

bool CLightWave::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{	
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;
	
	//copy image or animation
	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "LightWave");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	XMLNode nodeDrwable = nodeEffect.addChild("param");
	
	//get items
	if (animInfo.relative_filename_list.size() > 0)
	{
		std::string items = animInfo.relative_filename_list[0];
		items = items.replace(items.find("/") + 1, 6, "000000");
		nodeDrwable.addAttribute("Material", items.c_str());
	}
	
	char alpha[256];
	sprintf(alpha, "%.2f", m_Alltime);
	nodeDrwable.addAttribute("Alltime", alpha);

	sprintf(alpha, "%.2f", m_DuringTime);
	nodeDrwable.addAttribute("Onetime", alpha);

	sprintf(alpha, "%.2f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	sprintf(alpha, "%d", m_Loop);
	nodeDrwable.addAttribute("Loopnum", alpha);

	return true;
}

void CLightWave::SetEffectCount(int count)
{
	m_Loop = count;
	m_StartTime = GetRunTime();
}