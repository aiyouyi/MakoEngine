#include "CFaceLipstick.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/DXUtils/DX11Resource.h"

CFaceLipstick::CFaceLipstick()
{
	m_MouthMask = NULL;
	m_mouthManage = NULL;
	m_alpha = 1.0;
	m_EffectPart = FACE_LIPSTRICK;

	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
}


CFaceLipstick::~CFaceLipstick()
{
	Release();
}

void CFaceLipstick::Release()
{
	SAFE_DELETE(m_mouthManage);
	SAFE_DELETE(m_MouthMask);
	SAFE_DELETE(pTextureBlendWeight);
	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_DELETE(m_material);
}

void * CFaceLipstick::Clone()
{
	CFaceLipstick* result = new CFaceLipstick();
	*result = *this;
	return result;
}

bool CFaceLipstick::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode node= childNode.getChildNode("param", 0);
		if (!node.isEmpty())
		{
			const char *szMouthColor = node.getAttribute("mouthColor");
			if (szMouthColor != NULL)
			{
				sscanf(szMouthColor, "%f,%f,%f,%f", &m_MouthRGBA.x, &m_MouthRGBA.y, &m_MouthRGBA.z, &m_MouthRGBA.w);
				m_MouthRGBA /= 255.f;
			}
			const char *szgloss = node.getAttribute("gloss");
			if (szgloss != NULL)
			{
				sscanf(szgloss, "%f,%f,%f,%f", &m_gloss_contrast_scale, &m_gloss_contrast_shift, &m_gloss_contrast_shrink, &m_gloss_alpha);
			}

			const char *szbright = node.getAttribute("bright");
			if (szbright != NULL)
			{
				sscanf(szbright, "%f", &m_force_bright_threshold);
			}

			const char *szmetallight = node.getAttribute("metallight");
			if (szmetallight != NULL)
			{
				sscanf(szmetallight, "%f", &m_metallight_alpha);
			}

			const char *szSaturation = node.getAttribute("saturation");
			if (szSaturation)
			{
				sscanf(szSaturation, "%f", &m_saturation);
			}

			const char *szAlpha = node.getAttribute("teethalpha");
			if (szAlpha != NULL)
			{
				sscanf(szAlpha, "%f", &m_wihteTeethAlpha);
			}

			const char *szOpaque = node.getAttribute("alpha");
			if (szOpaque != NULL)
			{
				sscanf(szOpaque, "%f", &m_alpha);
			}

			const char *szDrawableName = node.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				//int w, h;
				//m_material = DXUtils::CreateTexFromZIP(hZip, szDrawableName,false);
				AnimInfo anim_info;
				anim_info.fps = 0;
				anim_info.dir = "";
				anim_info.relative_filename_list.push_back(szDrawableName);
				m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
				isWhitenTeeth = true;
			}
		}
		return true;
	}
	return false;
}

bool CFaceLipstick::ReadConfig(XMLNode& childNode, const std::string& path)
{
	CEffectPart::ReadConfig(childNode, path);
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("param", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			SAFE_DELETE(m_material);
			if (szDrawableName != NULL)
			{
				int w, h;
				std::string mask_path = path + "/" + szDrawableName;
				AnimInfo anim_info;
				anim_info.fps = 0;
				anim_info.dir = path;
				anim_info.relative_filename_list.push_back(szDrawableName);
				m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
				isWhitenTeeth = true;
			}
			const char *szAlpha = nodeDrawable.getAttribute("teethalpha");
			if (szAlpha != NULL)
			{
				sscanf(szAlpha, "%f", &m_wihteTeethAlpha);
			}

			const char *szOpaque = nodeDrawable.getAttribute("alpha");
			if (szOpaque != NULL)
			{
				sscanf(szOpaque, "%f", &m_alpha);
			}


			const char *szMouthColor = nodeDrawable.getAttribute("mouthColor");
			if (szMouthColor != NULL)
			{
				sscanf(szMouthColor, "%f,%f,%f,%f", &m_MouthRGBA.x, &m_MouthRGBA.y, &m_MouthRGBA.z, &m_MouthRGBA.w);
				m_MouthRGBA /= 255.f;
			}
			const char *szgloss = nodeDrawable.getAttribute("gloss");
			if (szgloss != NULL)
			{
				sscanf(szgloss, "%f,%f,%f,%f", &m_gloss_contrast_scale, &m_gloss_contrast_shift, &m_gloss_contrast_shrink, &m_gloss_alpha);
			}

			const char *szbright = nodeDrawable.getAttribute("bright");
			if (szbright != NULL)
			{
				sscanf(szbright, "%f", &m_force_bright_threshold);
			}

			const char *szmetallight = nodeDrawable.getAttribute("metallight");
			if (szmetallight != NULL)
			{
				sscanf(szmetallight, "%f", &m_metallight_alpha);
			}

			const char *szSaturation = nodeDrawable.getAttribute("saturation");
			if (szSaturation)
			{
				sscanf(szSaturation, "%f", &m_saturation);
			}

		}
		return true;
	}
	return false;
}

bool CFaceLipstick::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
	string path = m_resourcePath + "/Shader/Beauty/LipSticker.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	m_mouthManage = new MouthManage();
	byte_t blend_weight[256 * 4] = { 0,0,0,0,0,0,0,0,0,0,127,0,0,0,85,0,1,0,63,0,1,0,51,0,1,0,85,0,2,0,72,0,3,31,63,0,3,28,56,0,4,25,76,0,5,23,69,0,6,21,63,0,7,19,78,0,8,18,72,0,9,17,68,0,10,15,63,0,11,15,75,0,13,14,70,0,14,13,67,0,16,25,76,0,17,24,72,0,19,23,69,0,21,22,66,0,23,21,74,0,25,20,71,0,27,19,68,0,29,18,75,0,31,18,72,0,33,17,70,0,36,25,76,0,38,24,74,0,40,23,71,0,43,23,77,0,46,22,75,0,48,21,72,0,51,21,77,0,54,27,75,0,57,26,73,0,60,26,78,0,63,25,76,0,66,24,74,0,70,24,78,0,73,29,77,0,76,28,81,0,80,28,79,0,81,27,83,0,83,32,81,0,85,31,79,0,87,31,83,0,89,30,81,0,90,35,85,0,92,34,83,0,94,33,86,0,96,33,85,0,97,37,88,0,99,36,91,0,101,35,89,0,103,39,92,0,105,38,90,0,106,42,93,0,108,41,96,0,110,45,94,0,112,44,97,0,113,47,99,0,115,47,98,0,117,50,100,0,119,49,102,0,121,52,101,0,122,55,103,0,124,54,105,0,126,57,107,0,128,60,109,0,129,62,111,0,131,65,110,0,133,68,112,0,135,70,114,0,137,72,115,0,138,75,117,0,140,77,119,0,142,79,121,0,
									144,81,125,0,145,87,127,0,147,89,129,0,149,94,130,0,151,96,132,0,153,100,136,0,154,102,137,0,156,107,141,0,158,111,143,0,160,116,147,0,161,120,148,0,163,124,152,0,165,128,153,0,167,132,157,0,169,139,161,0,170,143,164,0,172,149,165,0,174,153,169,0,176,159,172,0,178,163,175,0,178,169,179,0,178,172,182,0,179,178,183,0,179,183,186,0,180,187,189,0,180,192,192,0,181,195,195,0,181,198,198,0,182,203,201,0,182,206,204,0,183,209,206,0,183,211,207,0,184,216,209,0,184,219,212,0,185,221,215,0,185,224,217,0,186,226,220,0,186,226,220,0,187,229,222,0,187,231,225,0,188,233,227,0,188,236,227,0,189,236,230,0,189,238,232,0,190,238,232,0,190,240,234,0,191,242,236,0,191,243,237,0,192,245,239,0,192,245,241,0,193,245,241,0,193,247,243,0,194,247,243,0,194,247,245,0,195,249,245,0,195,249,247,0,196,249,247,0,196,249,247,0,197,249,249,0,197,251,249,0,198,251,249,0,198,251,251,0,199,251,251,0,199,251,251,0,200,251,251,0,
									200,251,253,0,201,251,253,0,201,251,253,0,202,251,253,0,202,251,253,0,203,251,253,0,203,251,253,0,204,251,251,0,204,250,251,0,205,250,251,0,205,250,250,0,206,248,250,0,206,248,248,0,207,246,248,0,207,245,247,0,208,245,245,0,208,243,243,0,209,242,242,0,209,241,241,0,210,239,239,0,210,238,236,0,211,236,235,0,211,235,232,0,212,233,230,0,212,231,228,0,213,229,225,0,213,226,223,0,214,225,221,0,214,222,218,0,215,221,215,0,215,218,212,0,216,216,210,0,216,213,207,0,217,212,203,0,217,209,201,0,218,207,198,0,218,204,196,0,219,202,192,0,219,199,189,0,220,197,187,0,220,194,183,0,221,192,181,0,221,188,177,0,222,186,175,0,222,183,173,0,223,181,169,0,223,177,167,0,224,175,163,0,224,173,161,0,225,170,158,0,225,167,156,0,226,165,154,0,226,162,150,0,227,160,148,0,227,156,145,0,228,154,143,0,228,151,141,0,229,149,138,0,229,146,136,0,230,144,133,0,230,141,131,0,231,139,128,0,231,136,126,0,232,132,123,0,232,129,
									120,0,233,128,118,0,233,125,115,0,234,122,112,0,234,119,109,0,235,116,106,0,235,113,103,0,236,110,101,0,236,107,97,0,237,103,94,0,237,100,90,0,238,96,87,0,238,94,83,0,239,90,81,0,239,86,77,0,240,82,73,0,240,78,71,0,241,75,67,0,241,70,63,0,242,65,60,0,242,60,56,0,243,56,52,0,243,51,49,0,244,47,47,0,244,42,43,0,245,39,40,0,245,34,36,0,246,30,33,0,246,26,31,0,247,23,27,0,247,20,25,0,248,17,22,0,248,15,20,0,249,13,17,0,249,11,15,0,250,9,13,0,250,8,12,0,251,7,10,0,251,5,9,0,252,4,7,0,252,4,6,0,253,3,5,0,253,2,3,0,254,1,2,0,254,1,1,0,255,0,0,0 };

	pTextureBlendWeight = new DX11Texture();
	pTextureBlendWeight->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, 256, 1, blend_weight, 256 * 4, false);
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
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 16);
	}
	return true;
}

void CFaceLipstick::Render(BaseRenderParam &RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	if ( ResourceManager::Instance().getAnimFrame(m_anim_id, 0) == nullptr)
	{
		m_wihteTeethAlpha = 0;
	}
	if (m_MouthRGBA.w*m_alpha < 0.01 && m_wihteTeethAlpha < 0.01)
	{
		return;
	}


	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	float pParam[16];
	pParam[0] = m_MouthRGBA.x;
	pParam[1] = m_MouthRGBA.y;
	pParam[2] = m_MouthRGBA.z;
	pParam[3] = m_MouthRGBA.w*m_alpha;

	pParam[4] = m_gloss_contrast_scale;
	pParam[5] = m_gloss_contrast_shift;
	pParam[6] = m_gloss_contrast_shrink;
	pParam[7] = m_wihteTeethAlpha;;

	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->BindFBOA();

	m_pShader->useShader();

	auto pSrcShaderView = pTextureBlendWeight->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(2, 1, &pSrcShaderView);


	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_130);

		auto info = RenderParam.GetFaceInfo(faceIndex);

		m_mouthManage->GenMouthMask(pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight());

		SAFE_DELETE(m_MouthMask);
		m_MouthMask = new DX11Texture();
		m_MouthMask->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, m_mouthManage->m_nMaskWidth, m_mouthManage->m_nMaskHeight, m_mouthManage->m_pMouthMask, m_mouthManage->m_nMaskWidth * 4, false);
		pSrcShaderView = m_MouthMask->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);

		pParam[8] = m_mouthManage->m_Left*1.f / nWidth;
		pParam[9] = m_mouthManage->m_Top*1.f / nHeight;
		pParam[10] = (m_mouthManage->m_Right - m_mouthManage->m_Left)*1.f / nWidth;
		pParam[11] = (m_mouthManage->m_Bottom - m_mouthManage->m_Top)*1.f / nHeight;
		pParam[12] = m_saturation;

		float Verts[4];
		Verts[0] = pParam[8];
		Verts[1] = pParam[9];
		Verts[2] = m_mouthManage->m_Right*1.f / nWidth;
		Verts[3] = m_mouthManage->m_Bottom*1.f / nHeight;
		pDoubleBuffer->SyncAToBRegion(Verts, 2);
		//pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());


		Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, 0);
		if (img)
		{
			pSrcShaderView = RHIResourceCast(img->tex.get())->GetSRV();
			DeviceContextPtr->PSSetShaderResources(3, 1, &pSrcShaderView);
		}

		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	}
}

bool CFaceLipstick::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "FaceLipstick");
	nodeEffect.addAttribute("showname", m_showname.c_str());

	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);

	if (anim)
	{
		AnimInfo animInfo = anim->info;
		AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
		animInfo.relative_filename_list = renamed_info.relative_filename_list;

		XMLNode nodeDrwable = nodeEffect.addChild("param");
		nodeDrwable.addAttribute("Material", animInfo.relative_filename_list[0].c_str());

		char backAlphaInfo[256];
		sprintf(backAlphaInfo, "%.1f", m_wihteTeethAlpha);
		nodeDrwable.addAttribute("teethalpha", backAlphaInfo);
	}
	else
	{
		XMLNode nodeDrwable = nodeEffect.addChild("param");
		char mouthColor[256];
		sprintf(mouthColor, "%f,%f,%f,%f", m_MouthRGBA.x * 255.0, m_MouthRGBA.y * 255.0, m_MouthRGBA.z * 255.0, m_MouthRGBA.w * 255.0);
		nodeDrwable.addAttribute("mouthColor", mouthColor);

		char satu[256];
		sprintf(satu, "%f", m_saturation);
		nodeDrwable.addAttribute("saturation", satu);

		char opaque[256];
		sprintf(opaque, "%f", m_alpha);
		nodeDrwable.addAttribute("alpha", opaque);
	}

	return true;
}
