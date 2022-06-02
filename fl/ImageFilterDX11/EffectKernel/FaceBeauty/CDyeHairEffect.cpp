#include "CDyeHairEffect.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CDyeHairEffect::CDyeHairEffect()
{
    m_alpha = 1.0;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_pBlendState = NULL;
	m_EffectPart = DYE_HAIR_EFFECT;
}


CDyeHairEffect::~CDyeHairEffect()
{
    Release();
}

void CDyeHairEffect::Release()
{
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_pBlendState);
}

void * CDyeHairEffect::Clone()
{
	CDyeHairEffect* result = new CDyeHairEffect();
    *result = *this;
    return result;
}

bool CDyeHairEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CDyeHairEffect::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CDyeHairEffect::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	std::string  path = m_resourcePath + "/Shader/Beauty/DyeHair.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);
	//SAFE_RELEASE_BUFFER(m_pBlendStateNormal);
	//m_pBlendStateNormal = DXUtils::CloseBlendState();

	//m_pBlendState = DXUtils::CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA);

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
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 8);
	}

	return true;
}

bool CDyeHairEffect::IsEyeOpen(Vector2 *pFacePoint, float param)
{
	Vector2 pLeftEye[9];
	memcpy(pLeftEye, pFacePoint + 51, 9 * sizeof(Vector2));

	Vector2 pRightEye[9] = {
		pFacePoint[65],
		pFacePoint[64],
		pFacePoint[63],
		pFacePoint[62],
		pFacePoint[61],
		pFacePoint[68],
		pFacePoint[67],
		pFacePoint[66],
		pFacePoint[69],
	};

	//计算上下唇的距离相加取中值
	bool Ist = 0;
	//通过眼角角度计算睁眼闭眼
	Vector2 up = (pLeftEye[3] - pLeftEye[4]).normalize();
	Vector2 down = (pLeftEye[5] - pLeftEye[4]).normalize();

	//眼角角度
	float angle = 1.0 - up.dot(down);

	if (angle > param) {
		Ist = 1;
	}
	return Ist;
}

void CDyeHairEffect::Render(BaseRenderParam &RenderParam)
{
	auto hairTexture = RenderParam.GetHairMaskTexture();
	if (hairTexture == NULL)
	{
		return;
	}

	if (m_alpha < 0.001f)
	{
		return;
	}
	long runTime = GetRunTime();
	
	float pParam[8];
	pParam[0] = 0.0;//fill type
	pParam[1] = 0.0;
	//pParam[4] = m_HairRGBA.x;
	//pParam[5] = m_HairRGBA.y;
	//pParam[6] = m_HairRGBA.z;
	//pParam[7] = m_HairRGBA.w;

	pParam[4] = m_HairColor[m_CountColor].x;
	pParam[5] = m_HairColor[m_CountColor].y;
	pParam[6] = m_HairColor[m_CountColor].z;
	pParam[7] = 0.25;
	//if eye is open
	int nFaceCount = RenderParam.GetFaceCount();
	if(nFaceCount > 0)
	{
		//构建人脸变换矩阵信息
		FacePosInfo *pFaceInfo = RenderParam.GetFaceInfo(0);


		if (pFaceInfo->pFaceExp->blinkLeft && pFaceInfo->pFaceExp->blinkRight)
		{
			m_CountColor += 1;

			if (m_CountColor > 3)
			{
				m_CountColor = 0;
			}
		}
	}


	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SwapFBO();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	auto pMaskView = hairTexture->getTexShaderView();

	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetShaderResources(1, 1, &pMaskView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

bool CDyeHairEffect::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
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
			const char *szHairColor = nodeDrawable.getAttribute("hairColor");
			if (szHairColor != NULL)
			{
				sscanf(szHairColor, "%f,%f,%f,%f", &m_HairRGBA.x, &m_HairRGBA.y, &m_HairRGBA.z, &m_HairRGBA.w);
			}

		}
		return true;
	}
	return false;
}

bool CDyeHairEffect::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "DyeHair");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	XMLNode nodeDrwable = nodeEffect.addChild("param");
	
	char alpha[256];
	sprintf(alpha, "%.2f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	sprintf(alpha, "%.2f,%.2f,%.2f,%.2f", m_HairRGBA.x, m_HairRGBA.y, m_HairRGBA.z, m_HairRGBA.w);
	nodeDrwable.addAttribute("hairColor", alpha);


	return true;
}
