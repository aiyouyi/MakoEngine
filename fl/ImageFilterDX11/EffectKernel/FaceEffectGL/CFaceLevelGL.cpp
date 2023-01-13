#include "CFaceLevelGL.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"

CFaceLevelGL::CFaceLevelGL()
{
    m_alpha = 0;
	m_pFBO = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_EffectPart = FACE_LEVELS;
}


CFaceLevelGL::~CFaceLevelGL()
{
    Release();
}

void CFaceLevelGL::Release()
{
	m_pShader.reset();
	SAFE_DELETE(m_pFBO);
}

void * CFaceLevelGL::Clone()
{
	CFaceLevelGL* result = new CFaceLevelGL();
    *result = *this;
    return result;
}

bool CFaceLevelGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
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

bool CFaceLevelGL::Prepare()
{

	m_pShader = std::make_shared<class CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/faceLevel.vs";
    std::string  fspath = m_resourcePath + "/Shader/faceLevel.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    return true;
}

void CFaceLevelGL::Render(BaseRenderParam &RenderParam)
{
	if (m_alpha < 0.001f)
	{
		return;
	}
	int nWidth = RenderParam.GetWidth();
	int nHeight = RenderParam.GetHeight();

	//if (m_nWidth != nWidth || m_nHeight != nHeight)
	//{
	//	m_nWidth = nWidth;
	//	m_nHeight = nHeight;
	//	SAFE_DELETE(m_pFBO);
	//	m_pFBO = new CCFrameBuffer();
	//	m_pFBO->init(m_nWidth, m_nHeight);
	//}

    static const GLfloat g_TextureCoordinate[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

	pDoubleBuffer->SyncAToB();

	glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
	pDoubleBuffer->BindFBOA();
	m_pShader->Use();

	GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureB());
	m_pShader->SetUniform1i("inputImageTexture", 0);

	float normAlpha = m_alpha / 255.f;
	m_pShader->SetUniform1f("alpha", normAlpha);
	m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");

}
