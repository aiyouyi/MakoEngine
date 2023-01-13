#include "DoubleBuffer.h"
#include "BaseDefine/Define.h"
#include "Toolbox/GL/GLDynamicRHI.h"
#include "Toolbox/GL/GLTexture2D.h"
#include "CC3DEngine/Material/CC3DTexture.h"

GLDoubleBuffer::GLDoubleBuffer()
{
	m_nWidth = 0;
	m_nHeight = 0;
    m_filter = NULL;

}

GLDoubleBuffer::~GLDoubleBuffer()
{
    SAFE_DELETE(m_filter);
	this->Release();
}

void GLDoubleBuffer::Release()
{
}

void GLDoubleBuffer::InitDoubleBuffer(GLuint TexIDA,int nWidth, int nHeight,bool UseDepth )
{
    Release();
	m_FBOA = std::make_shared<GLTexture2D>();
	m_FBOB = std::make_shared<GLTexture2D>();

	m_FBOA->InitFrameBuffer(TexIDA,nWidth,nHeight,UseDepth);GL_CHECK();
	m_FBOB->InitFrameBuffer(nWidth,nHeight,UseDepth);GL_CHECK();
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void GLDoubleBuffer::InitDoubleBuffer(int nWidth, int nHeight, bool UseDepth, GLint internalformat, GLenum format, GLenum type)
{
	Release();
	m_FBOA = std::make_shared<GLTexture2D>();
	m_FBOB = std::make_shared<GLTexture2D>();
	m_FBOA->InitFrameBuffer(nWidth, nHeight, UseDepth, internalformat, format, type);
	m_FBOB->InitFrameBuffer(nWidth, nHeight, UseDepth, internalformat, format, type);
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}

void GLDoubleBuffer::InitDoubleBuffer(uint32_t format, int32_t width, int32_t height, bool UseDepth, bool UseMultiTarget,bool bMSAA)
{
	(bMSAA);
	auto [InternalFormat, FoundType, FoundFormat] = ConvertGLTextureFormat(format);
	if (InternalFormat == 0 || FoundType == 0 || FoundFormat == 0)
	{
		return ;
	}
	Release();
	m_FBOA = std::make_shared<GLTexture2D>();
	m_FBOB = std::make_shared<GLTexture2D>();
	m_FBOA->InitFrameBuffer(width, height, UseDepth, InternalFormat, FoundFormat, FoundType,UseMultiTarget);
	m_FBOB->InitFrameBuffer(width, height, UseDepth, InternalFormat, FoundFormat, FoundType,UseMultiTarget);
	m_nWidth = width;
	m_nHeight = width;
}

void GLDoubleBuffer::UpdtaeTexA(GLuint TexIDA)
{
    //m_pFBOA->textureID = TexIDA;
	m_FBOA->Get3DTexture()->m_FBO->textureID = TexIDA;
}

void GLDoubleBuffer::BindFBOA()
{
	m_FBOA->Get3DTexture()->bindFBO();
}

void GLDoubleBuffer::BindFBOB()
{
	m_FBOB->Get3DTexture()->bindFBO();
}

void GLDoubleBuffer::unBindFBOA()
{
	m_FBOA->Get3DTexture()->unbindFBO();
}

void GLDoubleBuffer::unBindFBOB()
{
    m_FBOB->Get3DTexture()->unbindFBO();
}


void GLDoubleBuffer::SwapFBO()
{
	std::swap(m_FBOA, m_FBOB);
}

void GLDoubleBuffer::FilterToTex(GLuint InptuTex)
{
     if(m_filter == NULL)
     {
         m_filter = new FilterBase ();
     }
 
     m_filter->FilterToTexture(InptuTex,m_nWidth,m_nHeight);

}

void GLDoubleBuffer::FilterToGray(GLuint InptuTex, int nWidth, int nHeight)
{
    if(m_filter == NULL)
    {
        m_filter = new FilterBase ();
    }

    m_filter->FilterToGray(InptuTex,nWidth,nHeight);
}

void GLDoubleBuffer::SyncAToB()
{
//     m_pFBOB->bind();
//     FilterToTex(GetFBOTextureA());
  //  m_pFBOB->unbind();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBOA->GetFrameBufferID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBOB->GetFrameBufferID());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBOB->GetTextureID(), 0);

	glBlitFramebuffer(0, 0, m_nWidth, m_nHeight, 0, 0, m_nWidth, m_nHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GLDoubleBuffer::SyncBToA()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBOB->GetFrameBufferID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBOA->GetFrameBufferID());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBOA->GetTextureID(), 0);

	glBlitFramebuffer(0, 0, m_nWidth, m_nHeight, 0, 0, m_nWidth, m_nHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void GLDoubleBuffer::SyncAToBRegion(float * pVerts, int nVerts, int nStep, int type)
{
	int left = m_nWidth;
	int right = 0;
	int top = m_nHeight;
	int bottom = 0;

	for (int i = 0; i < nVerts; i++)
	{
		float x = pVerts[0];
		float y = pVerts[1];
		pVerts += nStep;

		int xi, yi;
		if (type)
		{
			xi = (x*0.5 + 0.5)*m_nWidth;
			yi = (y*0.5 + 0.5)*m_nHeight;
		}
		else
		{
			xi = x * m_nWidth;
			yi = y * m_nHeight;
		}
		left = (std::min)(left, xi);
		right = (std::max)(right, xi);
		top = (std::min)(top, yi);
		bottom = (std::max)(bottom, yi);
	}
	left = (std::max)(left, 0);
	top = (std::max)(top, 0);


	right = (std::min)(right+1, (m_nWidth));// -left + 1;
	bottom = (std::min)(bottom+1, (m_nHeight));// -top + 1;
	GL_CHECK();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBOA->GetFrameBufferID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBOB->GetFrameBufferID());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBOB->GetTextureID(), 0);

	glBlitFramebuffer(left, top, right, bottom, left, top, right, bottom, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	GL_CHECK();
}

std::shared_ptr<CC3DTextureRHI>  GLDoubleBuffer::GetFBOTextureA()
{
	return m_FBOA;
}

std::shared_ptr<CC3DTextureRHI>  GLDoubleBuffer::GetFBOTextureB()
{
	return m_FBOB;
}

GLuint GLDoubleBuffer::GetFBOTextureAID()
{
	return m_FBOA->GetTextureID();
}

GLuint GLDoubleBuffer::GetFBOTextureBID()
{
	return m_FBOB->GetTextureID();
}

GLuint  GLDoubleBuffer::GetFBOA()
{
	return m_FBOA->GetFrameBufferID();
}

GLuint  GLDoubleBuffer::GetFBOB()
{
    return m_FBOB->GetFrameBufferID();
}

CCFrameBuffer* GLDoubleBuffer::GetFBOABuffer()
{
	return m_FBOA->Get3DTexture()->m_FBO;
}

CCFrameBuffer* GLDoubleBuffer::GetFBOBBuffer()
{
	return m_FBOB->Get3DTexture()->m_FBO;
}

int GLDoubleBuffer::GetWidth() const
{
	return m_nWidth;
}

int GLDoubleBuffer::GetHeight() const
{
	return m_nHeight;
}
