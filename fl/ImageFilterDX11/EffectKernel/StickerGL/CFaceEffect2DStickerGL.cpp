#include "CFaceEffect2DStickerGL.h"
#include "RenderGL/ShaderProgramManagerGL.h"
//#include "Algorithm/Matrices.h"
#include "Algorithm/MathUtils.h"
#include <algorithm>
#include "GL/GLTexture2D.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Drawable.hpp"

CFaceEffect2DStickerGL::CFaceEffect2DStickerGL()
{
	m_EffectPart = FACE_EFFECT_STICKER;
	m_pStickerShader = nullptr;
	m_Drawable = nullptr;
	VerticeBuffer = nullptr;
	RenderSrcRHI = nullptr;
	m_alpha = 1.0;
}

CFaceEffect2DStickerGL::~CFaceEffect2DStickerGL()
{
}

void CFaceEffect2DStickerGL::Release()
{
}

void * CFaceEffect2DStickerGL::Clone()
{
	CFaceEffect2DStickerGL* result = new CFaceEffect2DStickerGL();
	*result = *this;
	return result;
}

bool CFaceEffect2DStickerGL::ReadConfig(XMLNode & childNode, HZIP hZip, char * pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeRect = childNode.getChildNode("rect", 0);
		if (!nodeRect.isEmpty())
		{
			const char* szLeft = nodeRect.getAttribute("left");
			const char* szUp = nodeRect.getAttribute("up");
			const char* szRight = nodeRect.getAttribute("right");
			const char* szDown = nodeRect.getAttribute("down");
			if (szLeft != NULL && szUp != NULL && szRight != NULL && szDown != NULL)
			{
				float left = atof(szLeft);
				float up = atof(szUp);
				float right = atof(szRight);
				float down = atof(szDown);

				m_SrcRect[0] = Vector2(left, up);
				m_SrcRect[1] = Vector2(right, up);
				m_SrcRect[2] = Vector2(left, down);
				m_SrcRect[3] = Vector2(right, down);
			}


			const char* szShape = nodeRect.getAttribute("keepShpape");
			if (szShape != NULL)
			{
				m_KeepShape = atoi(szShape);
			}

		}
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
				sscanf(szLeftDown, "%f,%f", &m_SrcRect[2].x, &m_SrcRect[2].y);
				sscanf(szRightDown, "%f,%f", &m_SrcRect[3].x, &m_SrcRect[3].y);
				sscanf(szLeftUp, "%f,%f", &m_SrcRect[0].x, &m_SrcRect[0].y);
				sscanf(szRightUp, "%f,%f", &m_SrcRect[1].x, &m_SrcRect[1].y);
			}

			const char* szShape = nodePoint.getAttribute("keepShpape");
			if (szShape != NULL)
			{
				m_KeepShape = atoi(szShape);
			}

		}
		//drawable
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				//MaterialTex *pTex = new MaterialTex();
			    //pTex->CreateTexFromZIP(hZip, szDrawableName);
				std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
				m_Drawable = std::make_shared<BitmapDrawable>(TexRHI);
			}

			const char *szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);

		}
		//anidrawable image
		nodeDrawable = childNode.getChildNode("anidrawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("name");

			//CCAnimationDrawable *drawable = new CCAnimationDrawable();
			std::shared_ptr<AnimationDrawable> drawable = std::make_shared<AnimationDrawable>();

			const char *szOffset = nodeDrawable.getAttribute("offset");
			if (szOffset != NULL)
			{
				long offset = atol(szOffset);
				drawable->setOffset(offset);
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
					for (; iStart <= nCount; iStart += nStep)
					{
						sprintf(szImagePath, szItems, iStart);
						sprintf(szFullFile, "%s/%s", pFilePath, szImagePath);
						//MaterialTex *pTex = NULL;
						//if (pTex == NULL)
						//{
						//	pTex = new MaterialTex();
						//	pTex->CreateTexFromZIP(hZip, szImagePath);
						//}
						std::shared_ptr<MaterialTexRHI> TexRHI = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szImagePath);

						long during = nDuring;

						drawable->appandTex(during, TexRHI);
					}
				}
			}
			m_Drawable = drawable;
		}
		return true;
	}
	return false;
}

bool CFaceEffect2DStickerGL::Prepare()
{
	m_pStickerShader = make_shared<CCProgram>();
	std::string  vspath = m_resourcePath + "/Shader/faceEffectBlendMakeUp.vs";
	std::string  fspath = m_resourcePath + "/Shader/faceEffectBlendMakeUp.fs";
	m_pStickerShader->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
	GET_SHADER_STRUCT_MEMBER(FaceSticker).Shader_ = m_pStickerShader;
	m_nVerts = g_TextureCoordinate.size() / 2;
	//归一化坐标转换为绝对位置坐标
	for (int i = 0; i < 4; i++)
	{
		m_SrcRect[i].x = m_SrcRect[i].x * 930.f;
		m_SrcRect[i].y = m_SrcRect[i].y * 1240.f;
	}

	return true;
}

void CFaceEffect2DStickerGL::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	int nFaceCount = RenderParam.GetFaceCount();
	if (nFaceCount == 0)
	{
		return;
	}

	long runTime = RenderParam.runtime;
	if (runTime < 0)
		return;

	int width = RenderParam.GetWidth();
	int height = RenderParam.GetHeight();

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	m_pStickerShader->Use();
	GetDynamicRHI()->SetViewPort(0, 0, width, height);

	//m_pStickerShader->SetUniform1i("blendtype", m_BlendType);
	//m_pStickerShader->SetUniform1f("alpha", m_alpha);
	GET_SHADER_STRUCT_MEMBER(FaceSticker).SetParameter("alpha", m_alpha);
	GET_SHADER_STRUCT_MEMBER(FaceSticker).SetParameter("blendtype", static_cast<int32_t>(m_BlendType));

	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		Matrix3 warpMat;

		if (m_KeepShape)
		{
			float left = RenderParam.GetWidth() * 2;
			float right = -1000;
			float top = RenderParam.GetHeight() * 2;
			float bottom = -1000;
			float *pVerts = (float*)pFacePoint;
			for (int i = 0; i < 32; i++)
			{
				float x = pVerts[0];
				float y = pVerts[1];
				pVerts += 2;
				left = (std::min)(left, x);
				right = (std::max)(right, x);
				top = (std::min)(top, y);
				bottom = (std::max)(bottom, y);
			}
			m_DstPoint[0] = Vector2(left, top);
			m_DstPoint[1] = Vector2(left, bottom);
			m_DstPoint[2] = Vector2(right, bottom);
			warpMat = CMathUtils::getAffineTransform(m_SrcPointRect, m_DstPoint);
		}
		else
		{
			m_DstPoint[0] = pFacePoint[57];
			m_DstPoint[1] = pFacePoint[67];
			m_DstPoint[2] = pFacePoint[80];

			warpMat = CMathUtils::getAffineTransform(m_SrcPoint, m_DstPoint);
		}

		Vector2 m_Vertices[4];
		CMathUtils::transform((Vector2 *)m_SrcRect, (Vector2 *)m_Vertices, m_nVerts, warpMat);
		
		//顶点坐标归一化
		Vector2 ScaleSize(1.0 / width, 1.0 / height);
		for (int i = 0; i < 4; i++)
		{
			m_Vertices[i] = m_Vertices[i] * ScaleSize * 2.0 - 1.0;
		}
		if (!VerticeBuffer)
		{
			VerticeBuffer = GetDynamicRHI()->CreateVertexBuffer(2);
			VerticeBuffer->CreateVertexBuffer((float*)m_Vertices, m_nVerts, 2, 0);
			VerticeBuffer->CreateVertexBuffer(&g_TextureCoordinate[0], m_nVerts, 2, 1);
		}
		else
		{
			VerticeBuffer->UpdateVertexBUffer((float*)m_Vertices, m_nVerts, 0, 2 * sizeof(float), 0);
		}

		pDoubleBuffer->SyncAToBRegion((float*)m_Vertices, m_nVerts, 2, 1);
		pDoubleBuffer->BindFBOA();

		std::shared_ptr<MaterialTexRHI> TexRHI = m_Drawable->GetTex(runTime);
		if (!TexRHI)
		{
			continue;
		}
		TexRHI->Bind(0);
		m_pStickerShader->SetUniform1i("inputImageTexture", 0);
	
		RenderSrcRHI = pDoubleBuffer->GetFBOTextureB();
		GET_SHADER_STRUCT_MEMBER(FaceSticker).SetTexture("inputImageTexture2", RenderSrcRHI);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, pDoubleBuffer->GetFBOTextureB());
		//m_pStickerShader->SetUniform1i("inputImageTexture2", 1);
		
		GetDynamicRHI()->DrawPrimitive(VerticeBuffer);
	}


}