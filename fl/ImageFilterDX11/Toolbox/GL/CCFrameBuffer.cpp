#include "CCFrameBuffer.h"
#include <vector>


CCFrameBuffer::CCFrameBuffer()
{
    needReleaseTexture = false;
    needReleaseFramebuffer = false;
    onlyTexture = false;
    framebufferID = 0;
    textureID = 0;
    renderBufferID = 0;
    width = 0;
    height = 0;
    secondID = 0;
}

CCFrameBuffer::CCFrameBuffer(GLuint fbo, GLuint id, int nWidth, int nHeight)
{
    framebufferID = fbo;
    textureID = id;
    width = nWidth;
    height = nHeight;
    needReleaseTexture = false;
    needReleaseFramebuffer = false;
    secondID = 0;
}

CCFrameBuffer::~CCFrameBuffer()
{
    release();
}

void CCFrameBuffer::release()
{
    if(needReleaseTexture)
    {
        GL_DELETE_TEXTURE(textureID);
        GL_DELETE_TEXTURE(secondID);
    }
    if(needReleaseFramebuffer)
    {
        GL_DELETE_FRAMEBUFFER(framebufferID);
        GL_DELETE_RENDERBUFFER(renderBufferID);
    }
}

void CCFrameBuffer::bind()
{
    if(framebufferID!=0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
        if (secondID > 0)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondID, 0);
        }
        GL_CHECK()
    }
}

void CCFrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void CCFrameBuffer::init(int nWidth, int nHeight, bool bUseDepthBuffer, 
                         GLint internalformat, GLenum format, GLenum type, bool bUseMultiTarget ) {
    release();
    isMultiTarget = bUseMultiTarget;
    width = nWidth;
    height = nHeight;
    needReleaseFramebuffer = true;
    needReleaseTexture = true;
	generateTexture(internalformat, format, type);

    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
    if (isMultiTarget)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, secondID, 0);
    }

    if(bUseDepthBuffer)
    {
        glGenRenderbuffers(1, &renderBufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferID);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGE("ERROR: Incomplete filter FBO: %d; framebuffer size = %d, %d, glerror = %d, isTexture = %d, isFramebuffer = %d.", status, (int)width, (int)height, glGetError(), glIsTexture(textureID), glIsFramebuffer(framebufferID));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CCFrameBuffer::init(GLuint texture, int nWidth, int nHeight, bool bUseDepthBuffer){
    release();

    textureID = texture;
    width = nWidth;
    height = nHeight;

    isMultiTarget = false;
    needReleaseFramebuffer = true;
    needReleaseTexture = false;

    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    if(bUseDepthBuffer)
    {
        glGenRenderbuffers(1, &renderBufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBufferID);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGE("ERROR: Incomplete filter FBO: %d; framebuffer size = %d, %d, glerror = %d, isTexture = %d, isFramebuffer = %d.", status, (int)width, (int)height, glGetError(), glIsTexture(textureID), glIsFramebuffer(framebufferID));
    }
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
   // glBindTexture(GL_TEXTURE_2D, 0);
}

void CCFrameBuffer::ReadPixels(BYTE * pData)
{
	this->bind();
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pData);

	this->unbind();
}


void CCFrameBuffer::AttatchColorBuffers(int nCount)
{
	if (nCount < 1)
	{
		return;
	}

    if (!isMultiTarget)
    {
        nCount = 1;
    }

	std::vector<GLuint> attachments;
	for (int index = 0; index < nCount; ++index)
	{
		attachments.push_back(GL_COLOR_ATTACHMENT0 + index);
	}
	glDrawBuffers(attachments.size(), attachments.data());
}

void CCFrameBuffer::ClearColor()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CCFrameBuffer::ClearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

//void CCFrameBuffer::generateTexture()
//{
//    needReleaseTexture = true;
//    glGenTextures(1, &textureID);
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    // This is necessary for non-power-of-two textures
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)width, (int)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
//  //  glBindTexture(GL_TEXTURE_2D, 0);
//}

void CCFrameBuffer::generateTexture(GLint internalformat, GLenum format, GLenum type)
{
	needReleaseTexture = true;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// This is necessary for non-power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, (int)width, (int)height, 0, format, type, 0);
	//  glBindTexture(GL_TEXTURE_2D, 0);

    if (isMultiTarget)
    {
		glGenTextures(1, &secondID);
		glBindTexture(GL_TEXTURE_2D, secondID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// This is necessary for non-power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, (int)width, (int)height, 0, format, type, 0);
    }

}
