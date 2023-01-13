#include "CDenosieFilterGL.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"

CDeNosieFilterGL::CDeNosieFilterGL()
{
   m_pFBO = NULL;
   for(int i=0;i<NUM_RECORD_FBO;i++)
   {
       m_RecordTex[i] =0;
   }
   m_LastCovDoubleBuffer = NULL;
   m_pFBOLastResult = NULL;
   m_pShaderCov = NULL;
   m_nWidth = 0;
   m_nHeight = 0;
   m_EffectPart = DENOISE_EFFECT;
   m_alpha = 0.0;
}


CDeNosieFilterGL::~CDeNosieFilterGL()
{
    Release();
}

void CDeNosieFilterGL::Release()
{
    for(int i=0;i<NUM_RECORD_FBO;i++)
    {
        GL_DELETE_TEXTURE(m_RecordTex[i]);
    }

    SAFE_DELETE(m_pFBO);
    SAFE_DELETE(m_LastCovDoubleBuffer);
    SAFE_DELETE(m_pFBOLastResult);
    m_pShader.reset();
    m_pShaderCov.reset();
}

void * CDeNosieFilterGL::Clone()
{
    CDeNosieFilterGL* result = new CDeNosieFilterGL();
    *result = *this;
    return result;

}

bool CDeNosieFilterGL::Prepare()
{

    m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/LUTFilter.vs";
    std::string  fspath = m_resourcePath + "/Shader/DeNoise.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderCov = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/LUTFilter.vs";
    fspath = m_resourcePath + "/Shader/KGAndCov.fs";
    m_pShaderCov->CreatePorgramForFile(vspath.c_str(),fspath.c_str());
    return true;
}

bool CDeNosieFilterGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
    if (!childNode.isEmpty())
    {
        return true;
    }
    return false;
}

void CDeNosieFilterGL::Render(BaseRenderParam &RenderParam)
{
    if(m_alpha<0.001f)
    {
        return;
    }

    FilterKGAndCov(RenderParam);
    if(m_nFrame>6)
    {
        static const GLfloat g_TextureCoordinate[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
        auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
        glViewport(0,0,RenderParam.GetWidth(), RenderParam.GetHeight());
        pDoubleBuffer->BindFBOB();

        m_pShader->Use();

        GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureA());
        m_pShader->SetUniform1i("u_measureImageTexture", 0);
        glActiveTexture(GL_TEXTURE1);
        if(m_nFrame == 7)
        {
            glBindTexture(GL_TEXTURE_2D, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureAID());
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D,m_pFBOLastResult->textureID);

        }
        m_pShader->SetUniform1i("u_predictionImageTexture", 1);


        GetDynamicRHI()->SetPSShaderResource(2, m_LastCovDoubleBuffer->GetFBOTextureB());
        //glBindTexture(GL_TEXTURE_2D,m_RecordTex[5]);
        m_pShader->SetUniform1i("u_filterWeightImageTexture", 2);
        m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
       // m_pShader->SetUniform1f("u_fFilterWeight", m_alpha);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");

        pDoubleBuffer->SwapFBO();
        m_pFBOLastResult->bind();
        RHIResourceCast(pDoubleBuffer.get())->FilterToTex(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureAID());
    }
    m_nFrame ++;

}

void CDeNosieFilterGL::FilterKGAndCov(BaseRenderParam &RenderParam)
{
    static const GLfloat g_TextureCoordinate[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();

    int nWidth = pDoubleBuffer->GetWidth();
    int nHeight = pDoubleBuffer->GetHeight();

    int GrayWidth =  nWidth/3;
    int GrayHeight = nHeight/3;
    if(m_nWidth!=nWidth||m_nHeight != nHeight)
    {
        Release();
        m_nWidth = nWidth;
        m_nHeight = nHeight;
        m_LastCovDoubleBuffer = new GLDoubleBuffer();
        m_LastCovDoubleBuffer->InitDoubleBuffer(GrayWidth,GrayHeight);
        m_pFBOLastResult = new CCFrameBuffer();
        m_pFBOLastResult->init(nWidth,nHeight);
        m_pFBO = new CCFrameBuffer();
        m_nFrame =0;
        for (int i=0;i<NUM_RECORD_FBO;i++) {
            OpenGLUtil::createToTexture(m_RecordTex[i],GrayWidth,GrayHeight,0);
        }
        m_pFBO->init(m_RecordTex[0],GrayWidth,GrayHeight);

        m_LastCovDoubleBuffer->BindFBOB();
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    m_pFBO->textureID = m_RecordTex[m_nFrame%NUM_RECORD_FBO];
    m_pFBO->bind();
    RHIResourceCast(pDoubleBuffer.get())->FilterToGray(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureAID(),GrayWidth,GrayHeight);

    m_LastCovDoubleBuffer->BindFBOA();

    m_pShaderCov->Use();
    glViewport(0,0,GrayWidth, GrayHeight);

    std::string arrRecordTex[] = {"u_recordTexture0", "u_recordTexture1", "u_recordTexture2", "u_recordTexture3", "u_recordTexture4", "u_recordTexture5"};
    for(int i=0; i<NUM_RECORD_FBO; ++i)
    {
        glActiveTexture(GL_TEXTURE0+i);
        glBindTexture(GL_TEXTURE_2D, m_RecordTex[i]);
        m_pShaderCov->SetUniform1i(arrRecordTex[i].c_str(),i);
    }

    GetDynamicRHI()->SetPSShaderResource(6, m_LastCovDoubleBuffer->GetFBOTextureB());
    m_pShaderCov->SetUniform1i("u_lastCovTexture",6);
    m_pShaderCov->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_pShaderCov->DisableVertexAttribPointer("inputTextureCoordinate");
    m_LastCovDoubleBuffer->unBindFBOA();
    m_LastCovDoubleBuffer->SwapFBO();



}
