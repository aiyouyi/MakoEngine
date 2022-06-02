#include "CFaceShadowHighLight.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "Toolbox/DXUtils/DXUtils.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CFaceShadowHighLight::CFaceShadowHighLight()
{
	m_2DInterFace = NULL;
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;
	m_pConstantBuffer = NULL;
	m_pMergeVertex = NULL;
	m_EffectPart = SHADOW_HIGHLIGHT_EFFECT;
	m_alpha = 0.0;
}


CFaceShadowHighLight::~CFaceShadowHighLight()
{
	Release();
}

void CFaceShadowHighLight::Release()
{
	SAFE_DELETE(m_2DInterFace);	
	SAFE_RELEASE_BUFFER(m_pConstantBuffer);
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
	for (int i = 0; i < MAX_SUPPORT_PEOPLE; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}

	SAFE_DELETE_ARRAY(m_pMergeVertex);
}

void * CFaceShadowHighLight::Clone()
{
	CFaceShadowHighLight* result = new CFaceShadowHighLight();
	*result = *this;
	return result;
}

bool CFaceShadowHighLight::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");

	
	return false;
}

bool CFaceShadowHighLight::ReadConfig(XMLNode & childNode, const std::string & path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

long long CFaceShadowHighLight::GetImageID(HZIP hZip, const std::string & path, const char *szDrawableName)
{
	long long m_anim_id = 0;
	
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
	return m_anim_id;
}

bool CFaceShadowHighLight::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath, const std::string & path)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szmask = nodeDrawable.getAttribute("Mask");
			const char *szDrawableName = nodeDrawable.getAttribute("Normal");

			const char *szHL = nodeDrawable.getAttribute("Hightlight");
			const char *szSW = nodeDrawable.getAttribute("Shadow");
			if (szmask != NULL)
			{
				//int w, h;
				//m_mask = DXUtils::CreateTexFromZIP(hZip, szmask, false);
				m_anim_id = GetImageID(hZip, path, szmask);

			}
			if (szDrawableName != NULL)
			{
				//int w, h;
				//m_Normal = DXUtils::CreateTexFromZIP(hZip, szDrawableName, false);

				m_NormalId = GetImageID(hZip, path, szDrawableName);
			}
			if (szHL != NULL)
			{
				//int w, h;
				//m_Highlight = DXUtils::CreateTexFromZIP(hZip, szHL, false);

				m_HighLightID = GetImageID(hZip, path, szHL);
			}
			if (szSW != NULL)
			{
				//int w, h;
				//m_Shadowdow = DXUtils::CreateTexFromZIP(hZip, szSW, false);

				m_ShadowID = GetImageID(hZip, path, szSW);
			}

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

bool CFaceShadowHighLight::Prepare()
{
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};
	string path = m_resourcePath + "/Shader/Beauty/faceHighLight.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	if (m_pConstantBuffer == NULL)
	{
		m_pConstantBuffer = DXUtils::CreateConstantBuffer(sizeof(float) * 4);
	}

	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceShadowHighLight::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}

	float pParam[4];
	pParam[0] = m_alpha;

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SyncAToB();
	pDoubleBuffer->BindFBOA();
    m_pShader->useShader();

	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);

	auto pSrcShaderView = pDoubleBuffer->GetFBOTextureB()->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(0, 1, &pSrcShaderView);

	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, 0);
	pSrcShaderView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(1, 1, &pSrcShaderView);

	img = ResourceManager::Instance().getAnimFrame(m_NormalId, 0);
	pSrcShaderView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(2, 1, &pSrcShaderView);

	img = ResourceManager::Instance().getAnimFrame(m_HighLightID, 0);
	pSrcShaderView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(3, 1, &pSrcShaderView);

	img = ResourceManager::Instance().getAnimFrame(m_ShadowID, 0);
	pSrcShaderView = img->tex->getTexShaderView();
	DeviceContextPtr->PSSetShaderResources(4, 1, &pSrcShaderView);
	
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(m_pBlendStateNormal, blendFactor, 0xffffffff);
	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{

		auto info = RenderParam.GetFaceInfo(faceIndex);
		glm::mat4 rot_mtx_x = glm::rotate(glm::mat4(1.0f), (float)info->pitch*3.1416f / 180.0f, glm::vec3{ 1.0f, 0.0f, 0.0f });
		glm::mat4 rot_mtx_y = glm::rotate(glm::mat4(1.0f), (float)info->yaw*3.1416f / 180.0f, glm::vec3{ 0.0f, 1.0f, 0.0f });
		glm::mat4 rot_mtx_z = glm::rotate(glm::mat4(1.0f), (float)info->roll*3.1416f / 180.0f, glm::vec3{ 0.0f, 0.0f, 1.0f });

		glm::mat4 rot_mtx = rot_mtx_z * rot_mtx_x*rot_mtx_y;
		rot_mtx = glm::transpose(rot_mtx);
		pParam[1] = rot_mtx[2][0];
		pParam[2] = rot_mtx[2][1];
		pParam[3] = rot_mtx[2][2];
		DeviceContextPtr->UpdateSubresource(m_pConstantBuffer, 0, NULL, pParam, 0, 0);
		DeviceContextPtr->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		DeviceContextPtr->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_25D);
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


void CFaceShadowHighLight::MergeVertex(float * pVertex, float * pUV, int nVertex)
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

bool CFaceShadowHighLight::WriteConfig(std::string &tempPath, XMLNode &root, HZIP dst, HZIP src)
{
	//copy image or animation
	FileManager::Instance().SetSaveFolder(tempPath);
	
	Anim* anim = ResourceManager::Instance().getAnim(m_anim_id);
	
	AnimInfo animInfoMask = anim->info;
	AnimInfo renamed_info = FileManager::Instance().AddAnim(animInfoMask);
	animInfoMask.relative_filename_list = renamed_info.relative_filename_list;

	anim = ResourceManager::Instance().getAnim(m_NormalId);
	AnimInfo animInfoNormal = anim->info;
	renamed_info = FileManager::Instance().AddAnim(animInfoNormal);
	animInfoNormal.relative_filename_list = renamed_info.relative_filename_list;

	anim = ResourceManager::Instance().getAnim(m_HighLightID);
	AnimInfo animInfoHightLight = anim->info;
	renamed_info = FileManager::Instance().AddAnim(animInfoHightLight);
	animInfoHightLight.relative_filename_list = renamed_info.relative_filename_list;

	anim = ResourceManager::Instance().getAnim(m_ShadowID);
	AnimInfo animInfoShadow = anim->info;
	renamed_info = FileManager::Instance().AddAnim(animInfoShadow);
	animInfoShadow.relative_filename_list = renamed_info.relative_filename_list;


	XMLNode nodeEffect = root.addChild("typeeffect");
	nodeEffect.addAttribute("type", "FaceHighlight");
	nodeEffect.addAttribute("showname", m_showname.c_str());

	XMLNode nodeDrwable = nodeEffect.addChild("drawable");
	if (animInfoMask.relative_filename_list.size() > 0)
	{
		std::string items = animInfoMask.relative_filename_list[0];
		nodeDrwable.addAttribute("Mask", items.c_str());

		items = animInfoNormal.relative_filename_list[0];
		nodeDrwable.addAttribute("Normal", items.c_str());

		items = animInfoHightLight.relative_filename_list[0];
		nodeDrwable.addAttribute("Hightlight", items.c_str());

		items = animInfoShadow.relative_filename_list[0];
		nodeDrwable.addAttribute("Shadow", items.c_str());
	}

	char alpha[256];
	sprintf(alpha, "%.4f", m_alpha);
	nodeDrwable.addAttribute("alpha", alpha);

	return true;
}