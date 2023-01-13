#include "CFaceRemovePouchFalinGL.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/DoubleBuffer.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include <algorithm>

CFaceRemovePouchFalinGL::CFaceRemovePouchFalinGL()
{

    m_pFBO = NULL;
    m_nWidth = 0;
    m_nHeight = 0;
	m_DoubleBuffer = NULL;
    m_2DInterFace = NULL;
    m_EffectPart = FACE_REMOVE_FALIN;

    m_alpha = 0.0;
}


CFaceRemovePouchFalinGL::~CFaceRemovePouchFalinGL()
{
    Release();
}

void CFaceRemovePouchFalinGL::Release()
{
	m_pShaderSmooth.reset();
	m_pShaderMean.reset();
	m_pShader.reset();

    SAFE_DELETE(m_pFBO);
    SAFE_DELETE(m_2DInterFace);
	SAFE_DELETE(m_DoubleBuffer);
}

void * CFaceRemovePouchFalinGL::Clone()
{
	CFaceRemovePouchFalinGL* result = new CFaceRemovePouchFalinGL();
    *result = *this;
    return result;
}

bool CFaceRemovePouchFalinGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
    if (!childNode.isEmpty())
    {
        XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
        if (!nodeDrawable.isEmpty())
        {
            const char *szDrawableName = nodeDrawable.getAttribute("Material");
            if (szDrawableName != NULL)
            {
                int w,h;
				m_material = CreateTexFromZIP(hZip, szDrawableName, w, h);

            }

        }
        return true;
    }
    return false;
}

bool CFaceRemovePouchFalinGL::Prepare()
{
	m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/Anata/remove_falin_pouch.vs";
    std::string  fspath = m_resourcePath + "/Shader/Anata/remove_falin_pouch.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderSmooth = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/Anata/scale_blur.vs";
    fspath = m_resourcePath + "/Shader/Anata/scale_blur.fs";
    m_pShaderSmooth->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

	m_pShaderMean = std::make_shared<CCProgram>();
	vspath = m_resourcePath + "/Shader/Anata/scale.vs";
	fspath = m_resourcePath + "/Shader/Anata/scale.fs";
	m_pShaderMean->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
    m_2DInterFace = new mt3dface::MTFace2DInterFace();

    return true;
}

void CFaceRemovePouchFalinGL::Render(BaseRenderParam &RenderParam)
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
	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();



	if (m_nWidth != nWidth || m_nHeight != nHeight || m_DoubleBuffer->GetWidth() != m_nWidth * m_ScaleHW)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		SAFE_DELETE(m_pFBO);
		m_pFBO = new CCFrameBuffer();
		m_pFBO->init(m_nWidth*m_ScaleHW, m_nHeight*m_ScaleHW);
		SAFE_DELETE(m_DoubleBuffer);
		m_DoubleBuffer = new GLDoubleBuffer();
		m_DoubleBuffer->InitDoubleBuffer(m_nWidth*m_ScaleHW, m_nHeight*m_ScaleHW);
	}
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	if (m_alpha > 0.01 || m_pouchAlpha > 0.01)
	{
		static const GLfloat g_TextureCoordinate[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };

		glViewport(0, 0, m_DoubleBuffer->GetWidth(), m_DoubleBuffer->GetHeight());
		m_pFBO->bind();
		glClearColor(0.0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		m_pShaderMean->Use();

		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureA());
		m_pShaderMean->SetUniform1i("inputImageTexture", 0);
		m_pShaderMean->SetUniform2f("stepSize", 1.0 / nWidth, 1.0 / nHeight);
		for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
		{
			Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
			auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
			m_pShaderMean->SetVertexAttribPointer("position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
			glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle * 3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);
			m_pShaderMean->DisableVertexAttribPointer("position");
		}

		m_DoubleBuffer->BindFBOA();
		glClearColor(0.0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		m_pShaderSmooth->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pFBO->textureID);
		m_pShaderSmooth->SetUniform1i("inputImageTexture", 0);
		m_pShaderSmooth->SetVertexAttribPointer("position", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
		m_pShaderSmooth->SetUniform1f("texelWidthOffset", 0.f);
		m_pShaderSmooth->SetUniform1f("texelHeightOffset", 2.5 / pDoubleBuffer->GetHeight());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		m_pShaderSmooth->DisableVertexAttribPointer("position");

		m_DoubleBuffer->BindFBOB();
		glClearColor(0.0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		m_pShaderSmooth->Use();

		GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureA());
		m_pShaderSmooth->SetUniform1i("inputImageTexture", 0);
		m_pShaderSmooth->SetVertexAttribPointer("position", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
		m_pShaderSmooth->SetUniform1f("texelWidthOffset", 2.5f / pDoubleBuffer->GetWidth());
		m_pShaderSmooth->SetUniform1f("texelHeightOffset", 0.f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		m_pShaderSmooth->DisableVertexAttribPointer("position");
	}



	pDoubleBuffer->SyncAToB();
	glViewport(0, 0, pDoubleBuffer->GetWidth(), pDoubleBuffer->GetHeight());
	pDoubleBuffer->BindFBOA();
	m_pShader->Use();
	GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureB());
	m_pShader->SetUniform1i("inputImageTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pFBO->textureID);
	m_pShader->SetUniform1i("inputImageTexture2", 1);

	GetDynamicRHI()->SetPSShaderResource(2, m_DoubleBuffer->GetFBOTextureB());
	m_pShader->SetUniform1i("inputImageTexture3",2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_material);
	m_pShader->SetUniform1i("inputMaskTexture", 3);

	m_pShader->SetUniform1f("falinAlpha", m_alpha);
	m_pShader->SetUniform1f("pouchAlpha", m_pouchAlpha);
	m_pShader->SetUniform1f("BrightEyeAlpha", m_BrightEyeAlpha);

	for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
	{
		Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex, FACE_POINT_106);
		auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
		m_pShader->SetVertexAttribPointer("position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
		m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, (float*)FaceMesh->pTextureCoordinates);
		glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle * 3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);
		m_pShader->DisableVertexAttribPointer("position");
		m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");
	}
}
