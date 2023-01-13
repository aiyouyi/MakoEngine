#include "CFaceSmoothIns.h"
#include "BaseDefine/Define.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include <algorithm>
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceSmoothIns::CFaceSmoothIns()
{
    m_pShaderguide = NULL;
    m_pShaderSmooth = NULL;
    m_pFBO = NULL;
    m_pFBOFace = NULL;
    m_pShaderSkin = NULL;
	m_pShaderMean = NULL;
    m_pShaderFace = NULL;
    m_nWidth = 0;
    m_nHeight = 0;

    m_2DInterFace = NULL;
    m_material = NULL;
	m_DoubleBuffer = NULL;
	m_rectVerticeBuffer = NULL;
	m_rectIndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;
	m_alpha = 0.0;
	m_EffectPart = SMOOTH_EFFECT;
	m_pMergeVertex = NULL;
}


CFaceSmoothIns::~CFaceSmoothIns()
{
    Release();
}

void CFaceSmoothIns::Release()
{
    SAFE_DELETE(m_pFBO);
    SAFE_DELETE(m_pFBOFace);
    SAFE_DELETE(m_2DInterFace);
	SAFE_DELETE(m_material);

	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}

	SAFE_DELETE_ARRAY(m_pMergeVertex);
	SAFE_DELETE(m_DoubleBuffer);
}

void * CFaceSmoothIns::Clone()
{
    CFaceSmoothIns* result = new CFaceSmoothIns();
    *result = *this;
    return result;
}

bool CFaceSmoothIns::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
    if (!childNode.isEmpty())
    {
        XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
        if (!nodeDrawable.isEmpty())
        {
            const char *szDrawableName = nodeDrawable.getAttribute("Material");
			SAFE_DELETE(m_material);
            if (szDrawableName != NULL)
            {
                int w,h;
                //m_material = DXUtils::CreateTexFromZIP(hZip, szDrawableName,false);
				AnimInfo anim_info;
				anim_info.fps = 0;
				anim_info.dir = "";
				anim_info.relative_filename_list.push_back(szDrawableName);
				m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);

            }
			const char *szAlpha = nodeDrawable.getAttribute("smoothAlpha");;
			if (szAlpha != NULL)
			{
				sscanf(szAlpha, "%f,%f,%f", &m_TestAlpha[0], &m_TestAlpha[1], &m_TestAlpha[2]);
			}
			const char *szAlpha2 = nodeDrawable.getAttribute("backAlpha");
			if (szAlpha2 != NULL)
			{
				sscanf(szAlpha2, "%f", &m_BackAlpha);
			}

			const char *szAlpha3 = nodeDrawable.getAttribute("alpha");
			if (szAlpha3 != NULL)
			{
				sscanf(szAlpha3, "%f", &m_alpha);
			}

        }
        return true;
    }
    return false;
}

bool CFaceSmoothIns::ReadConfig(XMLNode& childNode, const std::string& path)
{
	CEffectPart::ReadConfig(childNode, path);
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
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

			}
			const char *szAlpha = nodeDrawable.getAttribute("smoothAlpha");
			if (szAlpha != NULL)
			{
				sscanf(szAlpha, "%f,%f,%f,%f", &m_TestAlpha[0], &m_TestAlpha[1], &m_TestAlpha[2]);
			}
			const char *szAlpha2 = nodeDrawable.getAttribute("backAlpha");
			if (szAlpha2 != NULL)
			{
				sscanf(szAlpha2, "%f", &m_BackAlpha);
			}

			const char *szAlpha3 = nodeDrawable.getAttribute("alpha");
			if (szAlpha3 != NULL)
			{
				sscanf(szAlpha3, "%f", &m_alpha);
			}
		}
		return true;
	}
	return false;
}

bool CFaceSmoothIns::Prepare()
{

	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};
    std::string  path = m_resourcePath + "/Shader/Beauty/InsBoxFilter.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	path = m_resourcePath + "/Shader/Beauty/guideFilter.fx";
	m_pShaderguide = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);


	path = m_resourcePath + "/Shader/Beauty/InsSmooth.fx";
	m_pShaderSmooth = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	path = m_resourcePath + "/Shader/Beauty/SkinSegment.fx";
	m_pShaderSkin = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	path = m_resourcePath + "/Shader/Beauty/InsMeanFilter.fx";
	m_pShaderMean = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	path = m_resourcePath + "/Shader/face2dEffectWithBG.fx";
	m_pShaderFace = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute2, 2);

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

    m_2DInterFace = new mt3dface::MTFace2DInterFace();

    return true;
}

void CFaceSmoothIns::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	float radius = 0.0036*m_TestAlpha[0];
	float radiusguide = 0.002*m_TestAlpha[2];

    if(RenderParam.GetFaceCount()>0)
    {
        auto info = RenderParam.GetFaceInfo(0);
		float rateFace = (std::max)(info->faceH / RenderParam.GetHeight(), info->faceW / RenderParam.GetWidth())*2.5;

		rateFace = (std::max)(0.25f, (std::min)(2.5f, rateFace));
		if (rateFace < 1.0)
		{
			radiusguide *= rateFace;
			float fScale = 1.f / rateFace;
			int iScale = (int)fScale;
			if (fScale - 0.1 > iScale)
			{
				iScale += 1;
			}
			m_ScaleHW = 0.25 *iScale;
		}
		else
		{
			m_ScaleHW = 0.25;
		}
		m_rateH = rateFace * 1.5;// info->faceH / RenderParam.GetHeight();
		m_rateW = m_rateH * RenderParam.GetHeight() / RenderParam.GetWidth();
    }

	float RadiusH = m_rateH * radius;
	float RadiusW = m_rateW * radius;

    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();


    FilterToSkinFBO(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureA(),RenderParam.GetWidth(), RenderParam.GetHeight());
    FilterToFaceFBO(RenderParam,RenderParam.GetWidth(), RenderParam.GetHeight());

	float pParam[4];
	pParam[0] = 0.05*m_TestAlpha[1];
	pParam[1] = radiusguide * RenderParam.GetHeight() / RenderParam.GetWidth();
	pParam[2] = radiusguide ;
	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	auto pBlendState = DXUtils::CloseBlendState();
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);

    m_DoubleBuffer->BindFBOB();
    m_pShaderguide->useShader();

	//auto pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureA());
	auto pSrcShaderView = m_pFBOFace->getTexture()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);

	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

	float maxRadius = (std::max)(RadiusW, RadiusH);
	if (maxRadius > 0.005)
	{
		RadiusW = RadiusW / maxRadius;
		RadiusH = RadiusH / maxRadius;
		RadiusW *= 0.005;
		RadiusH *= 0.005;
		maxRadius = maxRadius - 0.005;
		maxRadius = maxRadius / 0.005;

		RadiusW *= (1.0 + maxRadius * 0.33);
		RadiusH *= (1.0 + maxRadius * 0.33);

		maxRadius = maxRadius * 0.001;

		m_DoubleBuffer->BindFBOA();
		pParam[1] = maxRadius * RenderParam.GetHeight() / RenderParam.GetWidth();
		pParam[2] = 0;
		m_pShaderMean->useShader();
		//pSrcShaderView = m_DoubleBuffer->GetFBOTextureB()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureB());
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

		m_DoubleBuffer->BindFBOB();
		pParam[1] = 0;
		pParam[2] = maxRadius;
		m_pShaderMean->useShader();
		//pSrcShaderView = m_DoubleBuffer->GetFBOTextureA()->getTexShaderView();
		//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureA());
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	}



	pParam[1] = 0.f;
	pParam[2] = RadiusH;

    m_DoubleBuffer->BindFBOA();
    m_pShader->useShader();
	//pSrcShaderView = m_DoubleBuffer->GetFBOTextureB()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureB());
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);


	pParam[1] = RadiusW;
	pParam[2] = 0.f;;
	m_DoubleBuffer->BindFBOB();
    m_pShader->useShader();
	//pSrcShaderView = m_DoubleBuffer->GetFBOTextureA()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureA());
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);



	pParam[0] = m_alpha;
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);
    pDoubleBuffer->BindFBOB();
    m_pShaderSmooth->useShader();

	//pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureA());
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	
	//pSrcShaderView = m_DoubleBuffer->GetFBOTextureB()->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(1, m_DoubleBuffer->GetFBOTextureB());

	pSrcShaderView = m_pFBO->getTexture()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(2, 1, &pSrcShaderView);
	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);
	pDoubleBuffer->SwapFBO();
}

void CFaceSmoothIns::setAnim(AnimInfo& info)
{
	ResourceManager::Instance().freeAnim(m_anim_id);
	m_anim_id = ResourceManager::Instance().loadAnim(info);
}

bool CFaceSmoothIns::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "InsSmooth");
	nodeEffect.addAttribute("showname", m_showname.c_str());

	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	nodeDrwable.addAttribute("Material", animInfo.relative_filename_list[0].c_str());
	char alphaInfo[256];
	sprintf(alphaInfo, "%.1f,%.1f,%.1f", m_TestAlpha[0], m_TestAlpha[1], m_TestAlpha[2]);
	nodeDrwable.addAttribute("smoothAlpha", alphaInfo);
	char backAlphaInfo[256];
	sprintf(backAlphaInfo, "%.1f", m_BackAlpha);
	nodeDrwable.addAttribute("backAlpha", backAlphaInfo);
	char mAlpha[256];
	sprintf(mAlpha, "%.1f", m_alpha);
	nodeDrwable.addAttribute("alpha", mAlpha);

	return true;
}

void CFaceSmoothIns::FilterToSkinFBO(std::shared_ptr<CC3DTextureRHI> tex,int nWidth, int nHeight)
{
    if(m_nWidth!=nWidth||m_nHeight != nHeight|| m_DoubleBuffer->GetWidth() != m_nWidth * m_ScaleHW)
    {
        m_nWidth = nWidth;
        m_nHeight = nHeight;
        SAFE_DELETE(m_pFBO);
        SAFE_DELETE(m_pFBOFace);
		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(m_nWidth *m_ScaleHW, m_nHeight *m_ScaleHW, false, NULL);

		m_pFBOFace = new DX11FBO();
		m_pFBOFace->initWithTexture(m_nWidth *m_ScaleHW, m_nHeight *m_ScaleHW, false, NULL);

		SAFE_DELETE(m_DoubleBuffer);
		m_DoubleBuffer = new DX11DoubleBuffer;
		m_DoubleBuffer->InitDoubleBuffer(m_nWidth *m_ScaleHW, m_nHeight *m_ScaleHW);
    }
	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;
    m_pFBO->bind();
    m_pShaderSkin->useShader();
	//auto pSrcShaderView = tex->getTexShaderView();
	//DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	GetDynamicRHI()->SetPSShaderResource(0, tex);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
	DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);

}

void CFaceSmoothIns::FilterToFaceFBO(BaseRenderParam &RenderParam,int nWidth, int nHeight)
{
    int nFaceCount = RenderParam.GetFaceCount();
    if(nFaceCount<1)
    {
        return;
    }

    m_pFBOFace->bind();
	m_pFBOFace->clear(m_BackAlpha, 1.0, 1.0, 1.0);

    m_pShaderFace->useShader();
	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, 0);
	auto pMaterialView = RHIResourceCast(img->tex.get())->GetSRV();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
		MergeVertex(FaceMesh->pVertexs, FaceMesh->pTextureCoordinates, FaceMesh->nVertex);

		if (m_IndexBuffer == NULL)
		{
			m_IndexBuffer = DXUtils::CreateIndexBuffer(FaceMesh->pTriangleIndex, FaceMesh->nTriangle);
		}
		if (m_VerticeBuffer[faceIndex] == NULL)
		{
			m_VerticeBuffer[faceIndex] = DXUtils::CreateVertexBuffer(m_pMergeVertex, FaceMesh->nVertex, 5);
		}
		else
		{
			DXUtils::UpdateVertexBuffer(m_VerticeBuffer[faceIndex], m_pMergeVertex, FaceMesh->nVertex, 5 * sizeof(float), 5 * sizeof(float));
		}
		unsigned int nStride = (3 + 2) * sizeof(float);
		unsigned int nOffset = 0;
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[faceIndex], &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(FaceMesh->nTriangle * 3, 0, 0);
	}


}
void CFaceSmoothIns::MergeVertex(float * pVertex, float * pUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		m_pMergeVertex = new float[nVertex*(3 + 2)];
	}
	for (int i = 0; i < nVertex; i++)
	{
		m_pMergeVertex[i * 5] = pVertex[i * 3];
		m_pMergeVertex[i * 5 + 1] = pVertex[i * 3 + 1];
		m_pMergeVertex[i * 5 + 2] = pVertex[i * 3 + 2];
		m_pMergeVertex[i * 5 + 3] = pUV[i * 2];
		m_pMergeVertex[i * 5 + 4] = pUV[i * 2 + 1];
	}
}