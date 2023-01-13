#include "CBodyFGTrackEffect.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "BaseDefine/Define.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "EffectKernel/ResourceManager.h"
#include "EffectKernel/FileManager.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/VertexBuffer.h"
#include "Toolbox/RenderState/PiplelineState.h"


CBodyFGTrackEffect::CBodyFGTrackEffect()
{
	m_EffectPart = FORE_GROUND_TRACK;
	
	m_alpha = 1.0;
}

CBodyFGTrackEffect::~CBodyFGTrackEffect()
{
	Release();
}

void CBodyFGTrackEffect::Release()
{

}

void* CBodyFGTrackEffect::Clone()
{
	CBodyFGTrackEffect* result = new CBodyFGTrackEffect();
	*result = *this;
	return result;
}

void CBodyFGTrackEffect::Resize(int nWidth, int nHeight)
{
	if (m_Width != nWidth || m_Height != nHeight)
	{
		m_Width = nWidth;
		m_Height = nHeight;
	}
}

bool CBodyFGTrackEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath)
{
	CEffectPart::ReadConfig(childNode, hZip, pFilePath);
	ReadConfig(childNode, hZip, pFilePath, "");
	return true;
}

bool CBodyFGTrackEffect::ReadConfig(XMLNode& childNode, const std::string& path)
{
	CEffectPart::ReadConfig(childNode, path);
	ReadConfig(childNode, 0, NULL, path);
	return true;
}

void CBodyFGTrackEffect::ReadConfig(XMLNode& childNode, HZIP hZip, char* pFilePath, const std::string& path)
{
	if (!childNode.isEmpty())
	{
		//改用point字段
		XMLNode nodePoint = childNode.getChildNode("point", 0);
		if (!nodePoint.isEmpty())
		{

			const char* szLeftDown = nodePoint.getAttribute("leftdown");
			const char* szRightDown = nodePoint.getAttribute("rightdown");
			const char* szLeftUp = nodePoint.getAttribute("leftup");
			const char* szRightUp = nodePoint.getAttribute("rightup");
			if (szLeftDown != NULL && szRightDown != NULL && szLeftUp != NULL && szRightUp != NULL)
			{
				sscanf(szLeftDown, "%f,%f", &m_SrcRect[0].x, &m_SrcRect[0].y);
				sscanf(szRightDown, "%f,%f", &m_SrcRect[1].x, &m_SrcRect[1].y);
				sscanf(szLeftUp, "%f,%f", &m_SrcRect[2].x, &m_SrcRect[2].y);
				sscanf(szRightUp, "%f,%f", &m_SrcRect[3].x, &m_SrcRect[3].y);
			}

			//const char* szShape = nodePoint.getAttribute("keepShpape");
			//if (szShape != NULL)
			//{
			//	m_KeepShape = atoi(szShape);
			//}
		}

		//Time
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
			const char* szDrawableName = nodeDrawable.getAttribute("name");

			const char* szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
			//get items info
			const char* szItems = nodeDrawable.getAttribute("items");
			const char* szItemInfo = nodeDrawable.getAttribute("iteminfo");
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
		
		nodeDrawable = childNode.getChildNode("videodrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char* szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}

			const char* szRoate = nodeDrawable.getAttribute("DisableRoate");
			if (szRoate != NULL && !strcmp(szRoate, "true"))
			{
				m_DisableRoate = true;
			}

			const char* szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				if (hZip == 0)
				{
					VideoInfo video_info;
					video_info.dir = path;
					video_info.relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadVideo(video_info);
				}
				else
				{
					VideoInfo video_info;
					video_info.dir = path;
					video_info.relative_filepath = szDrawableName;
					m_anim_id = ResourceManager::Instance().loadVideoFromZip(video_info, hZip);
				}

			}
			const char* szEnableMp4Alpha = nodeDrawable.getAttribute("EnableMp4Alpha");
			if (szEnableMp4Alpha != NULL && !strcmp(szEnableMp4Alpha, "true"))
			{
				m_EnableMp4Alpha = true;
			}
		}
	}
}

bool CBodyFGTrackEffect::Prepare()
{
	m_Shader = GetDynamicRHI()->CreateShaderRHI();
	m_ShaderSplit = GetDynamicRHI()->CreateShaderRHI();
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C2},
			{VERTEX_ATTRIB_TEX_COORD, FLOAT_C2}
		};

		std::string fsPath = m_resourcePath + "/Shader/CBodyFGTrackEffect.fx";
		m_Shader->InitShader(fsPath, pAttribute, 2, true);


		CCVetexAttribute pAttribute1[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C2},
		};
		std::string fsSplitPath = m_resourcePath + "/Shader/splitScreen.fx";
		m_ShaderSplit->InitShader(fsSplitPath, pAttribute1, 1, false);

	}
	else
	{
		//To do GL
	}

	m_nVerts = g_TextureCoordinate.size() / 2;
	
	if (m_IndexBuffer == NULL)
	{
		unsigned short index[] =
		{
			0, 1, 2,
			1, 2, 3
		};

		m_IndexBuffer = GetDynamicRHI()->CreateIndexBuffer(index, 2);
	}

	if (m_SplitVerticeBuffer == nullptr)
	{
		m_SplitVerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(&g_TextureCoordinate[0], m_nVerts, 2);
	}

	return true;
}

void CBodyFGTrackEffect::Render(BaseRenderParam& RenderParam)
{
	if (m_alpha < 0.001f || RenderParam.GetBodyCount() <= 0)
	{
		return;
	}

	long runTime = GetRunTime();
	if (runTime < 0)
	{
		return;
	}

	Resize(RenderParam.GetWidth(), RenderParam.GetHeight());

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	m_Shader->UseShader();

	Image* img = ResourceManager::Instance().getImageCommon(m_anim_id, double(runTime));
	if (img == NULL)
	{
		return;
	}

	Vector2* BodyPoint = RenderParam.GetBodyPoint(0);
	m_DstPoint[0] = BodyPoint[12];
	m_DstPoint[1] = BodyPoint[9];
	m_DstPoint[2] = BodyPoint[13];

	Delta = m_DstPoint[2] - m_DstPoint[0];
	Delta.x /= m_Width;
	Delta.y /= m_Height;

	Vector4 pParam;
	pParam.x = m_alpha;
	pParam.y = GetBlendParm(m_BlendType);
	if (m_EnableMp4Alpha)pParam.z = 1.0; else pParam.z = 0.0;

	bool bMirror;
	int splitNum = RenderParam.GetSplitScreenNum(bMirror);
	Vector4 pClipParam = Vector4(0.0,1.0,1.0,0.0);
	static const float LerpK = 0.3;
	for (int j = 0; j < splitNum; j++)
	{
		pDoubleBuffer->BindFBOA();
		Vector2 dstPoint[3];
		
		for (int index = 0; index < 3; index++)
		{
			dstPoint[index].x = m_DstPoint[index].x + m_Width / splitNum * j;
			dstPoint[index].y = m_DstPoint[index].y; 

			dstPoint[index] = dstPoint[index] * LerpK + lastDstPoint[j][index] * (1.0 - LerpK);
			lastDstPoint[j][index] = dstPoint[index];
		}
		
		//warp transform
		WarpTransform(dstPoint);
		float A = Delta.length() > 0.07 ? 1 : 0;
		LastAlpha = pClipParam.z = LastAlpha * (1 - LerpK) + A * LerpK;
		pDoubleBuffer->SyncAToBRegion((float*)m_Vertices, m_nVerts, 2, 1);

		//变换后顶点坐标
		if (m_VerticeBuffer[j][0] == nullptr)
		{
			m_VerticeBuffer[j][0] = GetDynamicRHI()->CreateVertexBuffer((float*)m_Vertices, m_nVerts, 2);
		}
		else
		{
			m_VerticeBuffer[j][0]->UpdateVertexBUffer((float*)m_Vertices, m_nVerts, 2 * sizeof(float), 2 * sizeof(float));
		}

		//纹理坐标
		if (m_VerticeBuffer[j][1] == nullptr)
		{
			m_VerticeBuffer[j][1] = GetDynamicRHI()->CreateVertexBuffer(&g_TextureCoordinate[0], m_nVerts, 2);
		}

		pClipParam.x = j * 1.0;
		pClipParam.y = splitNum * 1.0;
		GET_SHADER_STRUCT_MEMBER(FGTrackConstBuffer).SetParameter("pClipParam", pClipParam);
		GET_SHADER_STRUCT_MEMBER(FGTrackConstBuffer).SetParameter("pParam", pParam);
		GET_SHADER_STRUCT_MEMBER(FGTrackConstBuffer).ApplyToAllBuffer();

		GetDynamicRHI()->SetPSShaderResource(0, img->tex);
		GetDynamicRHI()->SetPSShaderResource(1, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureB());

		GetDynamicRHI()->DrawPrimitive(m_VerticeBuffer[j], 2, m_IndexBuffer);

		pDoubleBuffer->unBindFBOA();   
	}
	
}

void CBodyFGTrackEffect::WarpTransform(Vector2 *dst)
{
	Matrix3 warpMat = CMathUtils::getAffineTransform(m_SrcPoint, dst);
	if (m_DisableRoate)
	{
		warpMat[1] = 0;
		warpMat[3] = 0;

		float scale_x = std::sqrt(warpMat[0] * warpMat[0] + warpMat[3] * warpMat[3]);
		float scale_y = (warpMat[0] * warpMat[4] - warpMat[1] * warpMat[3]) / scale_x;

		warpMat[0] = scale_x;
		warpMat[4] = scale_y;
	}
	for (int i = 0; i < m_nVerts; i++)
	{
		m_SrcAbsRect[i].x = m_SrcRect[i].x * m_ModelWidth;
		m_SrcAbsRect[i].y = m_SrcRect[i].y * m_ModelHeight;
	}


	for (int i = 0; i < m_nVerts; i++)
	{
		m_Vertices[i].x = m_SrcAbsRect[i].x * warpMat[0] + m_SrcAbsRect[i].y * warpMat[1] + warpMat[2];
		m_Vertices[i].y = m_SrcAbsRect[i].x * warpMat[3] + m_SrcAbsRect[i].y * warpMat[4] + warpMat[5];
	}
	//CMathUtils::transform((Vector2*)m_SrcAbsRect, (Vector2*)m_Vertices, m_nVerts, warpMat);

	Vector2 ScaleSize(1.0 / m_Width, 1.0 / m_Height);
	for (int i = 0; i < m_nVerts; i++)
	{
		m_Vertices[i] = m_Vertices[i] * ScaleSize * 2.0 - 1.0;
	}
}