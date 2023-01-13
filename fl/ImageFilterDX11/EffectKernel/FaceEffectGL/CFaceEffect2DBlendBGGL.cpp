#include "CFaceEffect2DBlendBGGL.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Drawable.hpp"
#include "RenderState/PiplelineState.h"

CFaceEffect2DBlendBGGL::CFaceEffect2DBlendBGGL()
{
	m_2DInterFace = NULL;
}


CFaceEffect2DBlendBGGL::~CFaceEffect2DBlendBGGL()
{
	Release();
}

void CFaceEffect2DBlendBGGL::Release()
{
	SAFE_DELETE(m_2DInterFace);
	m_pShader.reset();
}

void * CFaceEffect2DBlendBGGL::Clone()
{
	CFaceEffect2DBlendBGGL* result = new CFaceEffect2DBlendBGGL();
	*result = *this;
	return result;
}

bool CFaceEffect2DBlendBGGL::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
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

			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
			}
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
			const char *szBlendType = nodeDrawable.getAttribute("blendType");
			m_BlendType = GetBlendType(szBlendType);
			const char* szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				m_alpha = atof(szAlpha);
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

bool CFaceEffect2DBlendBGGL::Prepare()
{
	m_pShader = std::make_shared<class CCProgram>();
	
	std::string  vspath = m_resourcePath + "/Shader/face2dEffectBlendBG.vs";
	std::string  fspath = m_resourcePath + "/Shader/face2dEffectBlendBG.fs";


    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceEffect2DBlendBGGL::Render(BaseRenderParam& RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}

	if (RenderParam.GetFaceCount()==0)
	{
		return;
	}

	long runTime = RenderParam.runtime;
	if (runTime < 0)
		return;

	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();


    m_pShader->Use();
    glViewport(0,0,RenderParam.GetWidth(), RenderParam.GetHeight());

	int nFaceCount = RenderParam.GetFaceCount();
	int blendType = (int)m_BlendType;

	std::shared_ptr<MaterialTexRHI> TexRHI = m_Drawable->GetTex(runTime);
	if (TexRHI)
	{
		TexRHI->Bind(0);
		m_pShader->SetUniform1i("inputImageTexture", 0);
	}


	m_pShader->SetUniform1i("blendtype", blendType);
	m_pShader->SetUniform1f("alpha", m_alpha);

	GetDynamicRHI()->SetColorMask(1, 1, 1, 1);

	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);

		pDoubleBuffer->SyncAToBRegion((float*)FaceMesh->pVertexs, FaceMesh->nVertex, 3, 1);
		pDoubleBuffer->BindFBOA();


		GetDynamicRHI()->SetPSShaderResource(1, pDoubleBuffer->GetFBOTextureB());
		m_pShader->SetUniform1i("inputImageTexture2", 1);


		m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
		m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->pTextureCoordinates);
		glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle * 3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);

		//DrawTriLineToFBO((float*)FaceMesh->pVertexs, FaceMesh->nTriangle, FaceMesh->pTriangleIndex);
	}
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	GetDynamicRHI()->SetBlendState(CC3DPiplelineState::BSDisableWriteDisable, blendFactor, 0xffffffff);
}

bool CFaceEffect2DBlendBGGL::DrawTriLineToFBO(GLfloat* points, int count, unsigned short* indexs) {
	if (m_pShader == nullptr)
	{
		LOGE("m_pShader::DrawTriLineToFBO: program is nullptr !");
		return false;
	}


	unsigned short *lines = new unsigned short[count * 6];
	int lineCnt = count;
	unsigned short* pSrc = indexs;
	unsigned short* pDst = lines;
	for (int i = 0; i < lineCnt; i++) {
		pDst[0] = pSrc[0];
		pDst[1] = pSrc[1];
		pDst[2] = pSrc[1];
		pDst[3] = pSrc[2];
		pDst[4] = pSrc[2];
		pDst[5] = pSrc[0];
		pDst += 6;
		pSrc += 3;
	}
	glDrawElements(GL_LINES, lineCnt * 6, GL_UNSIGNED_SHORT, lines);
	m_pShader->DisableVertexAttribPointer("a_position");
	SAFE_DELETE_ARRAY(lines);
	return true;
}