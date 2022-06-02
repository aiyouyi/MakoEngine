#include "CLUTFIlter.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CLUTFIlter::CLUTFIlter()
{
    //m_material = 0;
    m_alpha = 0.0;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_EffectPart = LUT_FILTER_EFFECT;
}


CLUTFIlter::~CLUTFIlter()
{
    Release();
}

void CLUTFIlter::Release()
{
	//SAFE_DELETE(m_material);
	SAFE_DELETE(m_pFBO);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
}

void * CLUTFIlter::Clone()
{
    CLUTFIlter* result = new CLUTFIlter();
    *result = *this;
    return result;
}

bool CLUTFIlter::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CLUTFIlter::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CLUTFIlter::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	std::string  path = m_resourcePath + "/Shader/Beauty/LUTFilter.fx";
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
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	return true;
}

void CLUTFIlter::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}
	long runTime = GetRunTime();
	float pParam[4];
	pParam[0] = m_alpha;
	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SwapFBO();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	//pSrcShaderView = m_material->getTexShaderView();
	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, 0);
	pSrcShaderView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);

	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

bool CLUTFIlter::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
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

				}
				else
				{
					AnimInfo anim_info;
					anim_info.fps = 0;
					anim_info.dir = path;
					anim_info.relative_filename_list.push_back(szDrawableName);
					m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
				}
			}


			//SAFE_DELETE(m_material);
			//if (szDrawableName != NULL)
			//{
			//	int w, h;
			//	m_material = DXUtils::CreateTexFromZIP(hZip, szDrawableName, false);

			//}
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

bool CLUTFIlter::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "TYPEFilter");
	nodeEffect.addAttribute("showname", m_showname.c_str());
	////时间控制
	//XMLNode time = nodeEffect.addChild("time");

	XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	if (animInfo.relative_filename_list.size() > 0)
	{
		std::string items = animInfo.relative_filename_list[0];
		nodeDrwable.addAttribute("Material", items.c_str());
	}
	char alpha[256];
	sprintf(alpha, "%.4f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	return true;
}
