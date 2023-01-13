
#include "CC3DMakeUpGL.h"
#include "ToolBox/GL/GLResource.h"

CC3DMakeUpGL::CC3DMakeUpGL()
{
	m_FBO = NULL;
	m_pShader = NULL;
}

CC3DMakeUpGL::~CC3DMakeUpGL()
{
	SAFE_DELETE(m_FBO);
	SAFE_DELETE(m_pShader);
	for (int i=0;i<m_AllResource.size();i++)
	{
		SAFE_DELETE(m_AllResource[i].m_pTexture);
	}
	SAFE_DELETE(m_pShaderColor);
}

void CC3DMakeUpGL::initMainTexture(std::shared_ptr<CC3DTextureRHI>  pTexture, std::string path)
{
	m_pMainTexture = pTexture;

	if (m_FBO == NULL)
	{	
		m_FBO = new CCFrameBuffer();
		m_FBO->init(pTexture->GetWidth(), pTexture->GetHeight());;
	}

	if (m_pShader != NULL)
	{
		return;
	}
	m_pShader = new CCProgram();
	std::string  vspath = path + "/Shader/makeUp.vs";
	std::string  fspath = path + "/Shader/makeUp.fs";
	m_pShader->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

	m_pShaderColor = new CCProgram();
	vspath = path + "/Shader/ChangeColor.vs";
	fspath = path + "/Shader/ChangeColor.fs";
	m_pShaderColor->CreatePorgramForFile(vspath.c_str(), fspath.c_str());

}

void CC3DMakeUpGL::DeleteTexture(CC3DMakeUpType type)
{
	for (int i = 0; i < m_AllResource.size(); i++)
	{
		if (type == m_AllResource[i].type)
		{
			SAFE_DELETE(m_AllResource[i].m_pTexture);
			m_AllResource.erase(m_AllResource.begin() + i);
			break;
		}
	}
}

void CC3DMakeUpGL::PushTexture(BYTE * pRGBA, int nWidth, int nHeight, Vector2 * pRectVertices, CC3DMakeUpType type)
{
	if (pRGBA!=NULL)
	{
		CC3DTexture *pTexture = new CC3DTexture();
		pTexture->LoadTexture(pRGBA, nWidth, nHeight, true);
		CC3DMakeUpResource pResource;
		pResource.m_pTexture = pTexture;
		pResource.type = type;
		pResource.m_Vertices[0] = pRectVertices[0];
		pResource.m_Vertices[1] = pRectVertices[1];
		pResource.m_Vertices[2] = pRectVertices[2];
		pResource.m_Vertices[3] = pRectVertices[3];
		pResource.blendType = CC3D_NORAML;
		bool hasType = false;
		for (int i = 0; i < m_AllResource.size(); i++)
		{
			if (type == m_AllResource[i].type)
			{
				SAFE_DELETE(m_AllResource[i].m_pTexture);
				pResource.blendType = m_AllResource[i].blendType;
				m_AllResource[i] = pResource;
				hasType = true;
				break;
			}
		}
		if (!hasType)
		{
			m_AllResource.push_back(pResource);
		}

	}
}

void CC3DMakeUpGL::SetBlendType(CC3DMakeUpType type, CC3DBlendType blendType)
{
	for (int i = 0; i < m_AllResource.size(); i++)
	{
		if (m_AllResource[i].type == type)
		{
			m_AllResource[i].blendType = blendType;
		}
	}
}

void CC3DMakeUpGL::RenderMakeUp()
{
	if (m_NeedFlush && m_FBO != NULL)
	{
// 		ChangeColor(1, 0, 0, 1, m_pMainTexture);
// 
// 		ChangeColor(0, 0,1, 1, NULL, m_AllResource[0].type);

		m_FBO->bind();
		m_filter.FilterToTexture(RHIResourceCast(m_pMainTexture.get())->GetTextureID(), m_FBO->width, m_FBO->height);

		glViewport(0, 0, m_FBO->width, m_FBO->height);
		m_pShader->Use();
		static const float g_TextureCoordinate[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RHIResourceCast(m_pMainTexture.get())->GetTextureID());
		m_pShader->SetUniform1i("inputImageTexture2", 0);
		for (int i = 0; i < m_AllResource.size(); i++)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_AllResource[i].m_pTexture->GetGLTextureID());
			m_pShader->SetUniform1i("inputImageTexture", 1);
			m_pShader->SetUniform1i("blendtype", m_AllResource[i].blendType);
			m_pShader->SetUniform1f("alpha", 1.0);
			m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, m_AllResource[i].m_Vertices);
			m_pShader->SetVertexAttribPointer("a_texcoord", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}


}

void CC3DMakeUpGL::ChangeColor(float r, float g, float b, float a, CC3DTexture * pTexture, CC3DMakeUpType type)
{
	if (pTexture == NULL)
	{
		for (int i = 0; i < m_AllResource.size(); i++)
		{
			if (type == m_AllResource[i].type)
			{
				pTexture = m_AllResource[i].m_pTexture;
			}
		}
	}

	if (pTexture == NULL && m_pShaderColor == NULL)
	{
		return;
	}
	if (m_NeedFlush)
	{
		pTexture->bindFBO();

		glViewport(0, 0, pTexture->GetTextureWidth(), pTexture->GetTextureHeight());
		m_pShaderColor->Use();
		static const float g_TextureCoordinate[] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pTexture->m_pTextureLoad);
		m_pShaderColor->SetUniform1i("inputImageTexture", 0);
		m_pShader->SetVertexAttribPointer("a_position", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
		m_pShaderColor->SetUniform4f("HSVColors", r, g, b, a);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		pTexture->unbindFBO();
	}

}

void CC3DMakeUpGL::update()
{
	return;
}

