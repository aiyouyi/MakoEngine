#include "CFaceRemovePouchFalin.h"
#include "BaseDefine/Define.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include <algorithm>
#include "EffectKernel/ShaderProgramManager.h"

#include "../ResourceManager.h"
#include "../FileManager.h"

CFaceRemovePouchFalin::CFaceRemovePouchFalin()
{
    m_pShaderSmooth = NULL;
	m_pShaderMean = NULL;
    m_pFBO = NULL;
    m_nWidth = 0;
    m_nHeight = 0;
	m_DoubleBuffer = NULL;
    m_2DInterFace = NULL;
    m_EffectPart = FACE_REMOVE_FALIN;
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = nullptr;
	}

    m_alpha = 0.0;
}


CFaceRemovePouchFalin::~CFaceRemovePouchFalin()
{
    Release();
}

void CFaceRemovePouchFalin::Release()
{
    SAFE_DELETE(m_pFBO);
    SAFE_DELETE(m_2DInterFace);
	SAFE_DELETE(m_DoubleBuffer);

	SAFE_RELEASE_BUFFER(m_rectVerticeBuffer);
	SAFE_RELEASE_BUFFER(m_rectIndexBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);

	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}

	SAFE_DELETE_ARRAY(m_pMergeVertex);
}

void * CFaceRemovePouchFalin::Clone()
{
    CFaceRemovePouchFalin* result = new CFaceRemovePouchFalin();
    *result = *this;
    return result;
}

bool CFaceRemovePouchFalin::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");


	return false;
}

bool CFaceRemovePouchFalin::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

bool CFaceRemovePouchFalin::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
    if (!childNode.isEmpty())
    {
        XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
        if (!nodeDrawable.isEmpty())
        {
            const char *szDrawableName = nodeDrawable.getAttribute("Material");
            if (szDrawableName != NULL)
            {
                //int w,h;
				//m_material = DXUtils::CreateTexFromZIP(hZip, szDrawableName, false);
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
			const char *szAlpha = nodeDrawable.getAttribute("FaLingAlpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}

			szAlpha = nodeDrawable.getAttribute("PouchAlpha");
			if (szAlpha != NULL)
			{
				m_pouchAlpha = atof(szAlpha);
			}

			szAlpha = nodeDrawable.getAttribute("EyeBrightAlpha");
			if (szAlpha != NULL)
			{
				m_BrightEyeAlpha = atof(szAlpha);
			}

			const char *szRadius = nodeDrawable.getAttribute("FilterRadis");
			if (szRadius != NULL)
			{
				m_FilterRadis = atof(szRadius);
			}

        }
        return true;
    }
    return false;
}

bool CFaceRemovePouchFalin::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C2},
	};

	std::string path = m_resourcePath + "/Shader/Anata/scale_blur.fx";
	m_pShaderSmooth = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);


	path = m_resourcePath + "/Shader/Anata/scale.fx";
	m_pShaderMean = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 1);

	CCVetexAttribute pAttribute2[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};

	path = m_resourcePath + "/Shader/Anata/remove_falin_pouch.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute2, 2);

    m_2DInterFace = new mt3dface::MTFace2DInterFace();

    return true;
}

void CFaceRemovePouchFalin::Render(BaseRenderParam &RenderParam)
{
    if( RenderParam.GetFaceCount()<1)
    {
        return;
    }
	if (m_alpha < 0.01 && m_pouchAlpha < 0.01 && m_BrightEyeAlpha < 0.01)
	{
		return;
	}
	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount<1)
	{
		return;
	}
	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

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

	float pParam[4];
	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	if (m_nWidth != nWidth || m_nHeight != nHeight || m_DoubleBuffer->GetWidth() != m_nWidth * m_ScaleHW)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		SAFE_DELETE(m_pFBO);
		m_pFBO = new DX11FBO();
		m_pFBO->initWithTexture(m_nWidth *m_ScaleHW, m_nHeight *m_ScaleHW, false, NULL);
		SAFE_DELETE(m_DoubleBuffer);
		m_DoubleBuffer = new DX11DoubleBuffer();
		m_DoubleBuffer->InitDoubleBuffer(m_nWidth*m_ScaleHW, m_nHeight*m_ScaleHW);
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	unsigned int nStride = 2 * sizeof(float);
	unsigned int nOffset = 0;
	if (m_alpha > 0.01 || m_pouchAlpha > 0.01)
	{
		m_pFBO->bind();
		pParam[0] = 1.0 / nWidth ;
		pParam[1] =  1.0 / nHeight;

		m_pShaderMean->useShader();
		auto pSrcShaderView = pDoubleBuffer->GetFBOTextureA()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);


		pParam[0] = 0.f;
		pParam[1] = m_FilterRadis / pDoubleBuffer->GetHeight();
		m_DoubleBuffer->BindFBOA();
		m_pShaderSmooth->useShader();
	    pSrcShaderView = m_pFBO->getTexture()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);


		pParam[0] = m_FilterRadis / pDoubleBuffer->GetWidth();
		pParam[1] = 0.f;
		m_DoubleBuffer->BindFBOB();
		m_pShaderSmooth->useShader();
		pSrcShaderView = m_DoubleBuffer->GetFBOTextureA()->getTexShaderView();
		DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
		DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->IASetVertexBuffers(0, 1, &m_rectVerticeBuffer, &nStride, &nOffset);
		DeviceContextPtr->IASetIndexBuffer(m_rectIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		DeviceContextPtr->DrawIndexed(2 * 3, 0, 0);


	}

	pParam[0] = m_alpha * 0.8;
	pParam[1] = m_pouchAlpha * 0.8;
	pParam[2] = m_BrightEyeAlpha;
	pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();
	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	pSrcShaderView = m_pFBO->getTexture()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);
	pSrcShaderView = m_DoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(2, 1, &pSrcShaderView);

	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, 0);
	pSrcShaderView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(3, 1, &pSrcShaderView);

	DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
	DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

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


void CFaceRemovePouchFalin::MergeVertex(float * pVertex, float * pUV, int nVertex)
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

bool CFaceRemovePouchFalin::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	AnimInfo animInfo = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(anim->info);
	animInfo.relative_filename_list = renamed_info.relative_filename_list;

	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "RemovePouchFalin");
	nodeEffect.addAttribute("showname", m_showname.c_str());

	XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	if (animInfo.relative_filename_list.size() > 0)
	{
		std::string items = animInfo.relative_filename_list[0];
		nodeDrwable.addAttribute("Material", items.c_str());
	}
	char alpha[256];
	sprintf(alpha, "%.4f", m_alpha);
	nodeDrwable.addAttribute("FaLingAlpha", alpha);

	sprintf(alpha, "%.4f", m_pouchAlpha);
	nodeDrwable.addAttribute("PouchAlpha", alpha);

	sprintf(alpha, "%.4f", m_BrightEyeAlpha);
	nodeDrwable.addAttribute("EyeBrightAlpha", alpha);

	return true;
}