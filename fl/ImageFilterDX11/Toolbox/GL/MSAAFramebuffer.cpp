///-------------------------------------------------------------------------------------------------
///-------------------------------------------------------------------------------------------------
/// @file	E:\Git\ARGit\RealTimeMakeup\RealtimeMakeup\3DObject\MSAAFramebuffer.cpp.
///
/// @brief	Implements the msaa framebuffer class.
///-------------------------------------------------------------------------------------------------

#include "MSAAFramebuffer.h"
#include <string>
#include <vector>


MSAAFramebuffer::MSAAFramebuffer()
{
	m_isIpad = false;
	m_GLESVersion = 20;
	m_AsFrameBuffer = 0;
	m_AsFrameBufferTexture = 0;
	m_AsEmissTexture;
	m_SingleRenderBuffer = 0;
	m_SingleFramebuffer = 0;
	m_TextureWidth = 0;
	m_TextureHeight = 0;

	m_MSAAFramebuffer = 0;
	m_MSAARenderbuffer = 0;
	m_MSAADepthRenderbuffer = 0;
}

MSAAFramebuffer::~MSAAFramebuffer()
{
	this->Destory();
	m_AsFrameBufferTexture = 0;
	m_AsFrameBuffer = 0;
	m_AsEmissTexture = 0;
}

void MSAAFramebuffer::Destory()
{
	GL_DELETE_FRAMEBUFFER(m_MSAAFramebuffer);
	GL_DELETE_RENDERBUFFER(m_MSAARenderbuffer);
	GL_DELETE_RENDERBUFFER(m_MSAADepthRenderbuffer);
	GL_DELETE_FRAMEBUFFER(m_SingleFramebuffer);
	GL_DELETE_RENDERBUFFER(m_SingleRenderBuffer);

	GL_DELETE_TEXTURE(textureBufferMultiSample);
	GL_DELETE_TEXTURE(textureEmissMultiSample);
}


void MSAAFramebuffer::ClearColor()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MSAAFramebuffer::AttatchColorBuffers(int nCount)
{
	if (nCount < 1)
	{
		return;
	}
	std::vector<GLuint> attachments;
	for (int index = 0; index < nCount; ++index)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + index);
	}
	glDrawBuffers(attachments.size(), attachments.data());
}

bool MSAAFramebuffer::Initialize()
{
	this->Destory();
	//判断opengl版本
	m_GLESVersion = OpenGLUtil::GetOpenGLVersion();
	std::string renderDevice((const char*)glGetString(GL_RENDERER));
	if (renderDevice.find("PowerVR Rogue G6200") != std::string::npos) {
		m_GLESVersion = 20;
	}
	return true;

}

void MSAAFramebuffer::AsFrameBuffer(GLuint framebuffer)
{
	m_AsFrameBuffer = framebuffer;

}

void MSAAFramebuffer::AsFrameBufferTexture(GLuint framebufferTexture)
{
	m_AsFrameBufferTexture = framebufferTexture;
}

bool MSAAFramebuffer::SetupSize(int width, int height)
{
	if (width != m_TextureWidth || m_TextureHeight != height)
	{
		this->Destory();
		m_TextureWidth = width;
		m_TextureHeight = height;
	}
	return true;
}

bool MSAAFramebuffer::Bind()
{
	if (m_GLESVersion >= 30)
	{
		//gles 3.0+
		//glBindFramebuffer(GL_FRAMEBUFFER, m_AsFrameBuffer);
		//if (m_SingleRenderBuffer == 0)
		//{
		//	glGenRenderbuffers(1, &m_SingleRenderBuffer);
		//}
		//glBindRenderbuffer(GL_RENDERBUFFER, m_SingleRenderBuffer);
		////glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_AsFrameBufferTexture, 0);

		//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		//{
		//	LOGE("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		//	return false;
		//}
		//glEnable(GL_MULTISAMPLE);

		//多缓冲
		if (m_MSAAFramebuffer == 0)
		{
			glGenFramebuffers(1, &m_MSAAFramebuffer);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFramebuffer);

		//if (m_MSAARenderbuffer == 0)
		//{
		//	glGenRenderbuffers(1, &m_MSAARenderbuffer);
  //          glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARenderbuffer);
  //          // 4 samples for color
  //          int msci=4;
  //          if(m_TextureWidth>=1800||m_TextureHeight>=1800) msci=2;
  //          glRenderbufferStorageMultisample(GL_RENDERBUFFER, msci, GL_RGBA8, m_TextureWidth, m_TextureHeight);
		//}
		//glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARenderbuffer);
		//// 4 samples for color
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_MSAARenderbuffer);

		int msci = 4;
		if (m_TextureWidth >= 1800 || m_TextureHeight >= 1800)
			msci = 2;

		if (textureBufferMultiSample == 0)
		{
			if (!m_isIpad)
			{
				msci = 4;
				if (m_TextureWidth >= 1800 || m_TextureHeight >= 1800) msci = 2;
			}
			glGenRenderbuffers(1, &textureBufferMultiSample);
			glBindRenderbuffer(GL_RENDERBUFFER, textureBufferMultiSample);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msci, GL_RGBA16F, m_TextureWidth, m_TextureHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glGenRenderbuffers(1, &textureEmissMultiSample);
			glBindRenderbuffer(GL_RENDERBUFFER, textureEmissMultiSample);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msci, GL_RGBA16F, m_TextureWidth, m_TextureHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, textureBufferMultiSample);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, textureEmissMultiSample);

		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		if (m_MSAADepthRenderbuffer == 0)
		{
			if (!m_isIpad)
			{
				msci = 4;
				if (m_TextureWidth >= 1800 || m_TextureHeight >= 1800) msci = 2;
			}
			glGenRenderbuffers(1, &m_MSAADepthRenderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthRenderbuffer);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, msci, GL_DEPTH_COMPONENT24, m_TextureWidth, m_TextureHeight);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthRenderbuffer);
		// 4 samples for depth

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_MSAADepthRenderbuffer);

		// Test the framebuffer for completeness.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOGE("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			return false;
		}
		//bitblt
		if (m_isNeedDrawBack)
		{
			this->DrawBackGround();
		}

		return true;
	}
	else
	{
#ifdef PLATFORM_IOS
		//2.0+
		glBindFramebuffer(GL_FRAMEBUFFER, m_AsFrameBuffer);
		if (m_SingleRenderBuffer == 0)
		{
			glGenRenderbuffers(1, &m_SingleRenderBuffer);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, m_SingleRenderBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_AsFrameBufferTexture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOGE("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			return false;
		}

		//多缓冲
		if (m_MSAAFramebuffer == 0)
		{
			glGenFramebuffers(1, &m_MSAAFramebuffer);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFramebuffer);

		if (m_MSAARenderbuffer == 0)
		{
			glGenRenderbuffers(1, &m_MSAARenderbuffer);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARenderbuffer);
		glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_RGBA8_OES, m_TextureWidth, m_TextureHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_MSAARenderbuffer);

		if (m_MSAADepthRenderbuffer == 0)
		{
			glGenRenderbuffers(1, &m_MSAADepthRenderbuffer);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, m_MSAADepthRenderbuffer);
		glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24_OES, m_TextureWidth, m_TextureHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_MSAADepthRenderbuffer);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			LOGE("failed to make complete framebuffer object %x", status);
		}
		//bitblt
		this->DrawBackGround();

		return true;
#else
		//绑定内部framebuffer
		if (m_SingleFramebuffer == 0)
		{
			glGenFramebuffers(1, &m_SingleFramebuffer);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, m_SingleFramebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_AsFrameBufferTexture, 0);

		if (m_SingleRenderBuffer == 0)
		{
			glGenRenderbuffers(1, &m_SingleRenderBuffer);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, m_SingleRenderBuffer);

		GLenum renderbufferInternalformat = GL_DEPTH_COMPONENT24;

#ifdef PLATFORM_ANDROID
		const char* extString = (const char*)glGetString(GL_EXTENSIONS);
		renderbufferInternalformat = GL_DEPTH_COMPONENT16;
		if (strstr(extString, "GL_OES_depth24") != 0)
		{
			renderbufferInternalformat = GL_DEPTH_COMPONENT24;//GL_DEPTH_COMPONENT24_OES
		}
		else
		{
			LOGE("warning: current phone is not support GL_DEPTH_COMPONENT24_OES.");
		}
#endif
		glRenderbufferStorage(GL_RENDERBUFFER, renderbufferInternalformat, m_TextureWidth, m_TextureHeight);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_SingleRenderBuffer);

		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			LOGE("Create FrameBuffer error. ID = %d", status);
			return false;
		}

		return true;
#endif
	}
}

bool MSAAFramebuffer::Unbind(GLuint outputFbo)
{
	if (m_GLESVersion >= 30)
	{
		//glBindFramebuffer(GL_FRAMEBUFFER, m_AsFrameBuffer);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_AsFrameBufferTexture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_AsEmissTexture, 0);

		////GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		////glDrawBuffers(1, attachments);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outputFbo);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_AsFrameBufferTexture, 0);
		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_AsEmissTexture, 0);

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		//glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBlitFramebuffer(0, 0, m_TextureWidth, m_TextureHeight, 0, 0, m_TextureWidth, m_TextureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		glReadBuffer(GL_COLOR_ATTACHMENT1);

		GLuint attachments1[1] = { GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(1, attachments1);

		glBlitFramebuffer(0, 0, m_TextureWidth, m_TextureHeight, 0, 0, m_TextureWidth, m_TextureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//GL_CHECK();
//#ifdef PLATFORM_WINDOWS
//        if (m_GLESVersion >= 43)
//        {
//            //windows 4.3以上才支持
//            const GLenum discards1[]  = {GL_DEPTH_ATTACHMENT};
//            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards1);
//        }
//#else
//        const GLenum discards1[]  = {GL_DEPTH_ATTACHMENT};
//        glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards1);
//#endif

		// Copy the read fbo(multisampled framebuffer) to the draw fbo(single-sampled framebuffer)
	   // glBlitFramebuffer(0, 0, m_TextureWidth  , m_TextureHeight, 0, 0, m_TextureWidth  , m_TextureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		//glBlitFramebuffer(0, 0, m_TextureWidth  , m_TextureHeight, 0, 0, m_TextureWidth  , m_TextureHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
//#ifdef PLATFORM_WINDOWS
//        if (m_GLESVersion >= 43)
//        {
//            //windows 4.3以上才支持
//            const GLenum discards2[] = { GL_COLOR_ATTACHMENT0 };
//            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards2);
//
//        }
//#else
//        const GLenum discards2[]  = {GL_COLOR_ATTACHMENT0};
//        glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards2);
//
//#endif


		//GL_CHECK();
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}
	else
	{
#ifdef PLATFORM_IOS
		glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, m_MSAAFramebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, m_AsFrameBuffer);
		glResolveMultisampleFramebufferAPPLE();
		const GLenum discards[] = { GL_COLOR_ATTACHMENT0,GL_DEPTH_ATTACHMENT };
		glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, discards);
		glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, 0);
#else

#endif
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	return true;
}


void MSAAFramebuffer::DrawBackGround()
{
	// 	glViewport(0, 0, m_TextureWidth, m_TextureHeight);
	// 	//if (m_AsFrameBufferTexture == 0)
	// 	{
	// 
	// 		//----------------------------------------------------------
	// 		//画背景图
	// 		GLfloat texcoords[8] = { 0.0f, 1.0f,
	// 			1.0f, 1.0f,
	// 			0.0f, 0.0f,
	// 			1.0f, 0.0f };
	// 
	// 		GLfloat vertexs[8] = { 0.0f, (float)m_TextureHeight,
	// 			(float)m_TextureWidth, (float)m_TextureHeight,
	// 			0.0f, 0.0f,
	// 			(float)m_TextureWidth, 0 };
	// 
	// 		Matrix4 MvpMatrix = setOrthoFrustum(0.0f, m_TextureWidth + 0.0f, 0.0f, m_TextureHeight + 0.0f, -1.0f, 1.0f);
	// 		CGLProgram* progma = GLPOOL->Get(PROGRAM_SAMPLE);
	// 		progma->Use();
	// 
	// 		glActiveTexture(GL_TEXTURE0);
	// 		glBindTexture(GL_TEXTURE_2D, m_AsFrameBufferTexture);
	// 		progma->SetUniform1i("texture", 0);
	// 
	// 		progma->SetUniformMatrix4fv("mvpMatrix", MvpMatrix.getTranspose(), false, 1);
	// 		progma->SetVertexAttribPointer("position", 2, GL_FLOAT, false, 0, vertexs);
	// 		progma->SetVertexAttribPointer("texcoord", 2, GL_FLOAT, false, 0, texcoords);
	// 		//画背景
	//         //GL_CHECK();
	// 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	// 		progma->DisableVertexAttribPointer("position");
	// 		progma->DisableVertexAttribPointer("texCoord");
	// 
	// 		//----------------------------------------------------------
	// 	}
}

