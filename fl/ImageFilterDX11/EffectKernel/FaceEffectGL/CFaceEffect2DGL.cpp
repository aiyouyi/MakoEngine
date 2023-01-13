#include "CFaceEffect2DGL.h"
#include "BaseDefine/Define.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLDynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"

CFaceEffect2DGL::CFaceEffect2DGL()
{

	m_2DInterFace = NULL;
}


CFaceEffect2DGL::~CFaceEffect2DGL()
{
	Release();
}

void CFaceEffect2DGL::Release()
{
	SAFE_DELETE(m_2DInterFace);
	m_pShader.reset();
}

void * CFaceEffect2DGL::Clone()
{
	CFaceEffect2DGL* result = new CFaceEffect2DGL();
	*result = *this;
	return result;
}

bool CFaceEffect2DGL::ReadConfig(XMLNode & childNode, HZIP hZip ,char *pFilePath)
{
	if (!childNode.isEmpty())
	{
		XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
		if (!nodeDrawable.isEmpty())
		{
			const char *szDrawableName = nodeDrawable.getAttribute("Material");
			const char *szmask = nodeDrawable.getAttribute("Mask");
			if (szDrawableName != NULL)
			{
				m_material = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
			}
			if (szmask != NULL)
			{
                int w,h;
				m_mask = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szmask);
			}
		}
		return true;
	}
	return false;
}

bool CFaceEffect2DGL::Prepare()
{

	m_pShader = std::shared_ptr<CCProgram>(new CCProgram());
    std::string  vspath = m_resourcePath + "/Shader/face2dEffect.vs";
    std::string  fspath = m_resourcePath + "/Shader/face2dEffect.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());
	m_2DInterFace = new mt3dface::MTFace2DInterFace();
	return true;
}

void CFaceEffect2DGL::Render(BaseRenderParam& RenderParam)
{
	if (RenderParam.GetFaceCount() == 0)
	{
		return;
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->BindFBOA();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColorMask(1, 1, 1, 0);
    m_pShader->Use();
    glViewport(0,0,RenderParam.GetWidth(), RenderParam.GetHeight());
	m_material->Bind(0);
    m_pShader->SetUniform1i("inputImageTexture", 0);
    m_mask->Bind(1);
    m_pShader->SetUniform1i("inputImageTexture2", 1);


	int nFaceCount = RenderParam.GetFaceCount();
	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);

		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_25D);
		float *pMaskUV = m_2DInterFace->GetStandVerts();      
        m_pShader->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
        m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->pTextureCoordinates);
        m_pShader->SetVertexAttribPointer("a_texcoordMask", 2, GL_FLOAT, false, 0, (float*)pMaskUV);
        glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle*3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);
	}
    glDisable(GL_BLEND);
	glColorMask(1, 1, 1, 1);
}
