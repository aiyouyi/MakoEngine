#include "CFoodieSharpGL.h"
#include "Toolbox/GL/GLResource.h"

CFoodieSharpGL::CFoodieSharpGL()
{
	m_pShaderSharp = NULL;
	m_pFBO = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
    m_EffectPart = SHARP_EFFECT;
    m_alpha = 0.0;
}


CFoodieSharpGL::~CFoodieSharpGL()
{
    Release();
}

void CFoodieSharpGL::Release()
{

}

void * CFoodieSharpGL::Clone()
{
	CFoodieSharpGL* result = new CFoodieSharpGL();
    *result = *this;
    return result;
}

bool CFoodieSharpGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
    if (!childNode.isEmpty())
    {
        XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
        if (!nodeDrawable.isEmpty())
        {
            const char *szAlpha = nodeDrawable.getAttribute("Alpha");
            if(szAlpha!=NULL)
            {
                m_alpha = atof(szAlpha);
            }

        }
        return true;
    }
    return false;
}

bool CFoodieSharpGL::Prepare()
{

	m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/FoodieGauss.vs";
    std::string  fspath = m_resourcePath + "/Shader/FoodieGauss.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

	m_pShaderSharp = std::make_shared<CCProgram>();
	vspath = m_resourcePath + "/Shader/FoodieSharp.vs";
	fspath = m_resourcePath + "/Shader/FoodieSharp.fs";
	m_pShaderSharp->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
    return true;
}

void CFoodieSharpGL::Render(BaseRenderParam &RenderParam)
{
    if(m_alpha<0.001f)
    {
        return;
    }

	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

	if (m_nWidth != nWidth || m_nHeight != nHeight)
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		SAFE_DELETE(m_pFBO);
		m_pFBO = new CCFrameBuffer();
		m_pFBO->init(m_nWidth, m_nHeight);
	}

    static const GLfloat g_TextureCoordinate[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	float radius = 0.002;

	pDoubleBuffer->SwapFBO();

	glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
	pDoubleBuffer->BindFBOA();
	m_pShader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureBID());
	m_pShader->SetUniform1i("inputImageTexture", 0);
	m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	m_pShader->SetUniform1f("texelWidthOffset", 0.f);
	m_pShader->SetUniform1f("texelHeightOffset", radius);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");


	m_pFBO->bind();
	m_pShader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureAID());
	m_pShader->SetUniform1i("inputImageTexture", 0);
	m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
    m_pShader->SetUniform1f("texelWidthOffset", radius* RenderParam.GetHeight() / RenderParam.GetWidth());
	m_pShader->SetUniform1f("texelHeightOffset", 0.f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");

	pDoubleBuffer->BindFBOA();
	m_pShaderSharp->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureBID());
	m_pShaderSharp->SetUniform1i("inputImageTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pFBO->textureID);
	m_pShaderSharp->SetUniform1i("inputImageTexture2", 1);


	m_pShaderSharp->SetUniform1f("alpha", 1+m_alpha*0.4);

	m_pShaderSharp->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShaderSharp->DisableVertexAttribPointer("inputTextureCoordinate");

}
