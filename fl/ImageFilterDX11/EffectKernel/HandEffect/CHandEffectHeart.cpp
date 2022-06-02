#include "CHandEffectHeart.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include <iostream>
#include "EffectKernel/ShaderProgramManager.h"
#include "../ResourceManager.h"
#include "../FileManager.h"

CHandEffectHeart::CHandEffectHeart()
{
	m_pBox = NULL;
	m_pMergeVertex = NULL;


	for (int i = 0; i < MAX_SUPPORT_HAND; i++)
	{
		m_VerticeBuffer[i] = NULL;
	}
	m_IndexBuffer = NULL;

	isCrack = false;
	isCrackFinished = false;
}

CHandEffectHeart::~CHandEffectHeart()
{
	SAFE_DELETE(m_pBox);
	SAFE_DELETE_ARRAY(m_pMergeVertex);

	Release();
}

void CHandEffectHeart::Release()
{
	for (int i = 0; i < MAX_SUPPORT_HAND; i++)
	{
		SAFE_RELEASE_BUFFER(m_VerticeBuffer[i]);
	}
	SAFE_RELEASE_BUFFER(m_IndexBuffer);
}

bool CHandEffectHeart::ReadConfig(XMLNode & childNode, HZIP hZip, char *pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CHandEffectHeart::ReadConfig(XMLNode& childNode, const std::string &path)
{
	CEffectPart::ReadConfig(childNode);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

void CHandEffectHeart::ReadConfig(XMLNode& childNode, HZIP hZip, char *pFilePath, const std::string &path)
{
	if (!childNode.isEmpty())
	{
		//time
		XMLNode nodeTime = childNode.getChildNode("time", 0);
		if (!nodeTime.isEmpty())
		{
			const char* szDelay = nodeTime.getAttribute("delay");
			const char* szPlay = nodeTime.getAttribute("play");
			const char* szAll = nodeTime.getAttribute("all");
			if (szDelay != NULL)
			{
				m_play.delaytime = atoi(szDelay);
			}
			if (szPlay != NULL)
			{
				m_play.playtime = atoi(szPlay);
			}

			if (szAll != NULL)
			{
				m_play.alltime = atoi(szAll);
			}
		}
		
		//anidrawable
		XMLNode nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");

			const char *szType = nodeDrawable.getAttribute("EffectType");
			//get rect ratio
			const char *szBoxRate = nodeDrawable.getAttribute("rect");
			if (szBoxRate != NULL)
			{
				sscanf(szBoxRate, "%f,%f,%f,%f", &m_BoxRate[0], &m_BoxRate[1], &m_BoxRate[2], &m_BoxRate[3]);
			}
			//get items info
			const char *szItems = nodeDrawable.getAttribute("items");
			const char *szItemInfo = nodeDrawable.getAttribute("iteminfo");
			if (szItems != NULL && szItemInfo != NULL)
			{
				int iStart = 0;
				int nCount = 0;
				int nDuring = 0;
				int nStep = 1;
				char szImagePath[128];
				sscanf(szItemInfo, "%d,%d,%d,%d", &iStart, &nCount, &nDuring, &nStep);
				if (nCount >= 0 && nDuring > 0)
				{
					char szFullFile[256];
					AnimInfo anim_info;
					anim_info.fps = 1000.0 / float(nDuring);
					anim_info.dir = path;
					for (; iStart <= nCount; iStart += nStep)
					{
						sprintf(szImagePath, szItems, iStart);

						if (hZip == 0)
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
							anim_info.relative_filename_list.push_back(szImagePath);
						}
						else
						{
							sprintf(szFullFile, "%s/%s", path.c_str(), szImagePath);
							anim_info.relative_filename_list.push_back(szImagePath);
						}
					}
					if (hZip == 0)
					{
						m_anim_id = ResourceManager::Instance().loadAnim(anim_info);
					}
					else
					{
						m_anim_id = ResourceManager::Instance().loadAnimFromZip(anim_info, hZip);
					}
				}
			}
		}
	}
}

bool CHandEffectHeart::Prepare()
{
	//创建shader
	CCVetexAttribute pAttribute[] =
	{
		{VERTEX_ATTRIB_POSITION, FLOAT_C3},
		{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
	};

	std::string path = m_resourcePath + "/Shader/face2dTexture.fx";
	m_pShader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(path, pAttribute, 2);

	SAFE_RELEASE_BUFFER(m_pBlendStateNormal);
    m_pBlendStateNormal = DXUtils::CloseBlendState();

	return true;
}

void CHandEffectHeart::Render(BaseRenderParam &RenderParam)
{

	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}
	m_Width = RenderParam.GetWidth();
	m_Height = RenderParam.GetHeight();

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->BindFBOA();
	m_pShader->useShader();


	Image* img = ResourceManager::Instance().getAnimFrame(m_anim_id, float(runTime));
	auto pMaterialView = img->tex->getTexShaderView();
		
	DeviceContextPtr->PSSetSamplers(0, 1, &m_pSamplerLinear);
	DeviceContextPtr->PSSetShaderResources(0, 1, &pMaterialView);
		
	//Render fire
	std::vector<HandInfo> nHeartGesture;
	nHeartGesture = RenderParam.GetHeartGestureInfo();
	if (nHeartGesture.size() > 0)
	{
		for (int index = 0; index < nHeartGesture.size(); index++)
		{
			Vector2 *pHandPoint = nHeartGesture[index].handPoint;
			//生成比心框并判断是否和固定炮仗框有相交
			auto HandBox = GenerateHandBox(pHandPoint, m_BoxRate, 0);

			MergeVertex((float*)HandBox->m_pVertices, (float*)HandBox->m_pUV, HandBox->m_nVerts);

			if (m_IndexBuffer == NULL)
			{
				m_IndexBuffer = DXUtils::CreateIndexBuffer(HandBox->m_pTriangle, HandBox->m_nTriangle);
			}
			if (m_VerticeBuffer[index] == NULL)
			{
				m_VerticeBuffer[index] = DXUtils::CreateVertexBuffer(m_pMergeVertex, HandBox->m_nVerts, 5);
			}
			else
			{
				DXUtils::UpdateVertexBuffer(m_VerticeBuffer[index], m_pMergeVertex, HandBox->m_nVerts, 5 * sizeof(float), 5 * sizeof(float));
			}
			unsigned int nStride = (3 + 2) * sizeof(float);
			unsigned int nOffset = 0;

			DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VerticeBuffer[index], &nStride, &nOffset);
			DeviceContextPtr->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			DeviceContextPtr->DrawIndexed(HandBox->m_nTriangle * 3, 0, 0);
		}
	}
	
}

HandData *CHandEffectHeart::GenerateHandBox(Vector2 *pHandPoint, float* pBoxRate, int t)
{
	if (m_pBox == NULL)
	{
		m_pBox = new HandData();
	}
	int Width = m_Width;
	int Height = m_Height;

	Vector2 CenterPoint = (pHandPoint[0] + pHandPoint[4]) * 0.5;
	// 顶点
	Vector2 Vertices[4];
	float OffsetX = Width * pBoxRate[0];
	float OffsetY = Height * pBoxRate[2];
	Vertices[0] = Vector2(CenterPoint.x - OffsetX, CenterPoint.y);
	Vertices[1] = Vector2(CenterPoint.x + OffsetX, CenterPoint.y);
	Vertices[2] = Vector2(CenterPoint.x - OffsetX, CenterPoint.y - OffsetY);
	Vertices[3] = Vector2(CenterPoint.x + OffsetX, CenterPoint.y - OffsetY);
	
	////判断比心框和炮仗框是否重叠
	//if (m_vEffectInfo[t + 1].m_EffectType == STATIC_FIREWORK)
	//{
	//	IsRectsCross(Vertices, m_vEffectInfo[t + 1].m_BoxRate);
	//}
    
	Vector2 ScaleSize(1.0 / Width, 1.0 / Height);
	for (int i = 0; i < 4; i++)
	{
		Vertices[i] = Vertices[i] * ScaleSize * 2.0 - Vector2(1.0, 1.0);
	}
	m_pBox->m_nVerts = m_BoxUV.size() / 2;
	memcpy(m_pBox->m_pUV, (Vector2*)m_BoxUV.data(), sizeof(Vector2) * m_pBox->m_nVerts);
	memcpy(m_pBox->m_pVertices, Vertices, sizeof(Vector2) * m_pBox->m_nVerts);
	m_pBox->m_pTriangle = m_BoxIndex.data();
	m_pBox->m_nTriangle = m_BoxIndex.size() / 3;
	
	return m_pBox;

}

HandData *CHandEffectHeart::GenerateRect(float *pBoxRate)
{
	if (m_pBox == NULL)
	{
		m_pBox = new HandData();
	}
	int Width = m_Width;
	int Height = m_Height;

	// 顶点
	Vector2 Vertices[4];
	float left = Width * pBoxRate[0];
	float top = Height * pBoxRate[1];
	float right = Width * pBoxRate[2];
	float bottom = Height * pBoxRate[3];
	
	Vertices[0] = Vector2(right, top);
	Vertices[1] = Vector2(right, bottom);
	Vertices[2] = Vector2(left, bottom);
	Vertices[3] = Vector2(left, top);
	Vector2 ScaleSize(1.0 / Width, 1.0 / Height);
	for (int i = 0; i < 4; i++)
	{
		Vertices[i] = Vertices[i] * ScaleSize * 2.0 - Vector2(1.0, 1.0);

	}
	m_pBox->m_nVerts = m_BoxUV.size() / 2;
	memcpy(m_pBox->m_pUV, (Vector2*)m_BoxUV.data(), sizeof(Vector2) * m_pBox->m_nVerts);
	memcpy(m_pBox->m_pVertices, Vertices, sizeof(Vector2) * m_pBox->m_nVerts);
	m_pBox->m_pTriangle = m_BoxIndex.data();
	m_pBox->m_nTriangle = m_BoxIndex.size() / 3;

	return m_pBox;

}

void CHandEffectHeart::MergeVertex(float * pVertex, float * pUV, int nVertex)
{
	if (m_pMergeVertex == NULL)
	{
		int nSize = m_pBox->m_nVerts;
		m_pMergeVertex = new float[nSize*(3 + 2)];
	}
	for (int i = 0; i < nVertex; i++)
	{
		m_pMergeVertex[i * 5] = pVertex[i * 2];
		m_pMergeVertex[i * 5 + 1] = pVertex[i * 2 + 1];
		m_pMergeVertex[i * 5 + 2] = 0.5;
		m_pMergeVertex[i * 5 + 3] = pUV[i * 2];
		m_pMergeVertex[i * 5 + 4] = 1.0 - pUV[i * 2 + 1];
	}
}
