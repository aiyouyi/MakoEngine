#include "CFaceSmoothInsGL.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/CCProgram.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"


CFaceSmoothInsGL::CFaceSmoothInsGL()
{

    m_pFBO = NULL;
    m_pFBOFace = NULL;
    m_nWidth = 0;
    m_nHeight = 0;
	m_DoubleBuffer = NULL;
    m_2DInterFace = NULL;
    m_EffectPart = SMOOTH_EFFECT;

    m_alpha = 0.0;
}


CFaceSmoothInsGL::~CFaceSmoothInsGL()
{
    Release();
}

void CFaceSmoothInsGL::Release()
{
    SAFE_DELETE(m_pFBO);
    SAFE_DELETE(m_pFBOFace);
    SAFE_DELETE(m_2DInterFace);
	SAFE_DELETE(m_DoubleBuffer);

	m_pShader.reset();
	m_pShaderguide.reset();
	m_pShaderSmooth.reset();
	m_pShaderMean.reset();
	m_pShaderFace.reset();
	m_pShaderSkin.reset();
}

void * CFaceSmoothInsGL::Clone()
{
	CFaceSmoothInsGL* result = new CFaceSmoothInsGL();
    *result = *this;
    return result;
}

bool CFaceSmoothInsGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
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
				m_material = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);

            }
			const char *szAlpha = nodeDrawable.getAttribute("alpha");
			if (szAlpha != NULL)
			{
				sscanf(szAlpha, "%f,%f,%f,%f", &m_TestAlpha[0], &m_TestAlpha[1], &m_TestAlpha[2]);
			}
			const char *szAlpha2 = nodeDrawable.getAttribute("backAlpha");
			if (szAlpha2 != NULL)
			{
				sscanf(szAlpha2, "%f", &m_BackAlpha);
			}

        }
        return true;
    }
    return false;
}

bool CFaceSmoothInsGL::Prepare()
{

	m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/InsBoxFilter.vs";
    std::string  fspath = m_resourcePath + "/Shader/InsBoxFilter.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderguide = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/guideFilter.vs";
    fspath = m_resourcePath + "/Shader/guideFilter.fs";
    m_pShaderguide->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderSmooth = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/InsSmooth.vs";
    fspath = m_resourcePath + "/Shader/InsSmooth.fs";
    m_pShaderSmooth->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderSkin = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/SkinSegment.vs";
    fspath = m_resourcePath + "/Shader/SkinSegment.fs";
    m_pShaderSkin->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

    m_pShaderFace = std::make_shared<CCProgram>();
    vspath = m_resourcePath + "/Shader/face2dEffectWithBG.vs";
    fspath = m_resourcePath + "/Shader/face2dEffectWithBG.fs";
    m_pShaderFace->CreatePorgramForFile(vspath.c_str(),fspath.c_str());

	m_pShaderMean = std::make_shared<CCProgram>();
	vspath = m_resourcePath + "/Shader/InsMeanFilter.vs";
	fspath = m_resourcePath + "/Shader/InsMeanFilter.fs";
	m_pShaderMean->CreatePorgramForFile(vspath.c_str(), fspath.c_str());
    m_2DInterFace = new mt3dface::MTFace2DInterFace();

    return true;
}

void CFaceSmoothInsGL::Render(BaseRenderParam &RenderParam)
{

    if(m_alpha<0.001f)
    {
        return;
    }

	static const GLfloat g_TextureCoordinate[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
	float radius = 0.0036*m_TestAlpha[0];
	float radiusguide = 0.002*m_TestAlpha[2];

	if (RenderParam.GetFaceCount() > 0)
	{
		auto info = RenderParam.GetFaceInfo(0);

		float rateFace = (std::max)(info->faceH / RenderParam.GetHeight(), info->faceW / RenderParam.GetWidth())*2.5;

		rateFace = (std::max)(0.25f, (std::min)(2.5f, rateFace));
		if (rateFace < 1.0)
		{
			radiusguide *= rateFace;
			float fScale = 1.f / rateFace;
			int iScale = (int)fScale;
			if (fScale - 0.1 > iScale)
			{
				iScale += 1;
			}
			m_ScaleHW = 0.25 *iScale;
		}
		else
		{
			m_ScaleHW = 0.25;
		}
		m_rateH = rateFace * 1.5;// info->faceH / RenderParam.GetHeight();
		m_rateW = m_rateH * RenderParam.GetHeight() / RenderParam.GetWidth();
	}
	float RadiusH = m_rateH * radius;
	float RadiusW = m_rateW * radius;
	auto pDoubleBuffer = RenderParam.GetDoubleBuffer();


	FilterToSkinFBO(RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureAID(), RenderParam.GetWidth(), RenderParam.GetHeight());
	FilterToFaceFBO(RenderParam, RenderParam.GetWidth(), RenderParam.GetHeight());

	glViewport(0, 0, m_DoubleBuffer->GetWidth(), m_DoubleBuffer->GetHeight());

	m_DoubleBuffer->BindFBOB();
	m_pShaderguide->Use();

	GetDynamicRHI()->SetPSShaderResource(0, pDoubleBuffer->GetFBOTextureA());
	m_pShaderguide->SetUniform1i("inputImageTexture", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_pFBOFace->textureID);
	m_pShaderguide->SetUniform1i("inputImageTextureFace", 1);

	m_pShaderguide->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	m_pShaderguide->SetUniform1f("texelWidthOffset", radiusguide* RenderParam.GetHeight() / RenderParam.GetWidth());
	m_pShaderguide->SetUniform1f("texelHeightOffset", radiusguide);
	m_pShaderguide->SetUniform1f("alpha", 0.05*m_TestAlpha[1]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShaderguide->DisableVertexAttribPointer("inputTextureCoordinate");


	float maxRadius = (std::max)(RadiusW, RadiusH);
	if (maxRadius > 0.005)
	{
		RadiusW = RadiusW / maxRadius;
		RadiusH = RadiusH / maxRadius;
		RadiusW *= 0.005;
		RadiusH *= 0.005;
		maxRadius = maxRadius - 0.005;
		maxRadius = maxRadius / 0.005;

		RadiusW *= (1.0 + maxRadius * 0.33);
		RadiusH *= (1.0 + maxRadius * 0.33);

		maxRadius = maxRadius * 0.001;
		m_DoubleBuffer->BindFBOA();
		m_pShaderMean->Use();
		GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureB());
		m_pShaderMean->SetUniform1i("inputImageTexture", 0);
		m_pShaderMean->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
		m_pShaderMean->SetUniform1f("texelWidthOffset", 0.f);
		m_pShaderMean->SetUniform1f("texelHeightOffset", maxRadius);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		m_pShaderMean->DisableVertexAttribPointer("inputTextureCoordinate");

		m_DoubleBuffer->BindFBOB();
		m_pShaderMean->Use();
		GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureA());
		m_pShaderMean->SetUniform1i("inputImageTexture", 0);
		m_pShaderMean->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
		m_pShaderMean->SetUniform1f("texelWidthOffset", maxRadius*RenderParam.GetHeight() / RenderParam.GetWidth());
		m_pShaderMean->SetUniform1f("texelHeightOffset", 0.f);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		m_pShaderMean->DisableVertexAttribPointer("inputTextureCoordinate");

	}

	m_DoubleBuffer->BindFBOA();
	m_pShader->Use();
	GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureB());
	m_pShader->SetUniform1i("inputImageTexture", 0);
	m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	m_pShader->SetUniform1f("texelWidthOffset", 0.f);
	m_pShader->SetUniform1f("texelHeightOffset", RadiusH);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");



	m_DoubleBuffer->BindFBOB();
	m_pShader->Use();
	GetDynamicRHI()->SetPSShaderResource(0, m_DoubleBuffer->GetFBOTextureA());
	m_pShader->SetUniform1i("inputImageTexture", 0);
	m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	m_pShader->SetUniform1f("texelWidthOffset", RadiusW);
	m_pShader->SetUniform1f("texelHeightOffset", 0.f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");


    glViewport(0, 0, RenderParam.GetWidth(), RenderParam.GetHeight());
	pDoubleBuffer->BindFBOA();
	m_pShaderSmooth->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,  RHIResourceCast(RenderParam.GetSrcTex().get())->GetTextureID());
	m_pShaderSmooth->SetUniform1i("inputImageTexture", 0);

	GetDynamicRHI()->SetPSShaderResource(1, m_DoubleBuffer->GetFBOTextureB());
	m_pShaderSmooth->SetUniform1i("inputImageTexture2", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_pFBO->textureID);
	m_pShaderSmooth->SetUniform1i("inputImageTextureSkin", 2);

	//     glActiveTexture(GL_TEXTURE3);
	//     glBindTexture(GL_TEXTURE_2D, m_pFBOFace->textureID);
	//     m_pShaderSmooth->SetUniform1i("inputImageTextureFace", 3);

	m_pShaderSmooth->SetUniform1f("alpha", m_alpha);

	m_pShaderSmooth->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_pShaderSmooth->DisableVertexAttribPointer("inputTextureCoordinate");

}

void CFaceSmoothInsGL::FilterToSkinFBO(GLuint tex,int nWidth, int nHeight)
{
    if(m_nWidth!=nWidth||m_nHeight != nHeight)
    {
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		SAFE_DELETE(m_pFBO);
		SAFE_DELETE(m_pFBOFace);
		m_pFBO = new CCFrameBuffer();
		m_pFBO->init(m_nWidth*m_ScaleHW, m_nHeight*m_ScaleHW);
		m_pFBOFace = new CCFrameBuffer();
		m_pFBOFace->init(m_nWidth*m_ScaleHW, m_nHeight*m_ScaleHW);
		SAFE_DELETE(m_DoubleBuffer);
		m_DoubleBuffer = new GLDoubleBuffer();
		m_DoubleBuffer->InitDoubleBuffer(m_nWidth*m_ScaleHW, m_nHeight*m_ScaleHW);
    }

    glViewport(0,0,m_pFBO->width, m_pFBO->height);
    static const GLfloat g_TextureCoordinate2[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
    m_pFBO->bind();
    m_pShaderSkin->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    m_pShaderSkin->SetUniform1i("inputImageTexture", 0);
    m_pShaderSkin->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_pShaderSkin->DisableVertexAttribPointer("inputTextureCoordinate");

}

void CFaceSmoothInsGL::FilterToFaceFBO(BaseRenderParam &RenderParam,int nWidth, int nHeight)
{
    int nFaceCount = RenderParam.GetFaceCount();
    if(nFaceCount<1)
    {
        return;
    }

    glViewport(0,0,m_pFBO->width, m_pFBO->height);
    m_pFBOFace->bind();
	glClearColor(m_BackAlpha, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    m_pShaderFace->Use();
	m_material->Bind(0);
    m_pShaderFace->SetUniform1i("inputImageTexture", 0);
    for (int faceIndex = 0; faceIndex < nFaceCount; faceIndex++)
    {
        Vector2 *pFacePoint = RenderParam.GetFacePoint(faceIndex,FACE_POINT_106);
        auto FaceMesh = m_2DInterFace->Get2DMesh((float*)pFacePoint, RenderParam.GetWidth(), RenderParam.GetHeight(), 0, mt3dface::MT_FACE_2D_BACKGROUND);
        m_pShaderFace->SetVertexAttribPointer("a_position", 3, GL_FLOAT, false, 0, (float*)FaceMesh->pVertexs);
        m_pShaderFace->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, (float*)FaceMesh->pTextureCoordinates);
        glDrawElements(GL_TRIANGLES, FaceMesh->nTriangle*3, GL_UNSIGNED_SHORT, FaceMesh->pTriangleIndex);
        m_pShaderFace->DisableVertexAttribPointer("a_position");
        m_pShaderFace->DisableVertexAttribPointer("a_texcoord");
    }

}
