#include "CFaceEffect2DWithBGGL.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLDynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CFaceEffect2DWithBGGL::CFaceEffect2DWithBGGL()
{

	m_2DInterFace = NULL;
}


CFaceEffect2DWithBGGL::~CFaceEffect2DWithBGGL()
{
	Release();
}

void CFaceEffect2DWithBGGL::Release()
{
	SAFE_DELETE(m_2DInterFace);
	m_pShader.reset();
	m_material.reset();
}

void * CFaceEffect2DWithBGGL::Clone()
{
	CFaceEffect2DWithBGGL* result = new CFaceEffect2DWithBGGL();
	*result = *this;
	return result;
}

bool CFaceEffect2DWithBGGL::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			if (szDrawableName != NULL)
			{
				m_material = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
			}
		}
		return true;
	}
	return false;
}

bool CFaceEffect2DWithBGGL::Prepare()
{
	m_pShader = std::make_shared<class CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/face2dEffectWithBG.vs";
    std::string  fspath = m_resourcePath + "/Shader/face2dEffectWithBG.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceEffect2DWithBGGL::Render(BaseRenderParam &RenderParam)
{
	if (RenderParam.GetFaceCount()==0)
	{
		return;
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
    pDoubleBuffer->BindFBOA();
	glColorMask(1, 1, 1, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    m_pShader->Use();
    glViewport(0,0,RenderParam.GetWidth(), RenderParam.GetHeight());
	m_material->Bind(0);
    m_pShader->SetUniform1i("inputImageTexture", 0);

	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex,FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
        m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
        m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->pTextureCoordinates);
        glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle*3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);
	}

    glDisable(GL_BLEND);
	glColorMask(1, 1, 1, 1);
}

