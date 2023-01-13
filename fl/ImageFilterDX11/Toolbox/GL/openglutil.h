#ifndef OPENGLUTIL_H
#define OPENGLUTIL_H

#include"BaseDefine/Define.h"

#if defined(PLATFORM_ANDROID)
#ifndef ARKERNEL_NO_GL3STUB
//#include "Android/gl3stub.h"
#endif
#endif

#include "DX11ImageFilterDef.h"

#include"stdio.h"

//安全删除gl资源
#define GL_DELETE_TEXTURE(x) if(x) { glDeleteTextures(1,&x); x = 0; }

#define GL_DELETE_PROGRAM(x) if(x) { glDeleteProgram(x); x = 0;}

#define GL_DELETE_FRAMEBUFFER(x) if(x) { glDeleteFramebuffers(1,&x); x = 0;}

#define GL_DELETE_RENDERBUFFER(x) if(x) { glDeleteRenderbuffers(1,&x); x = 0;}

#define GL_DELETE_BUFFER(x) if(x) { glDeleteBuffers(1,&x); x = 0;}

#define GL_DELETE_VAO(x) if(x) { glDeleteVertexArrays(1,&x); x = 0;}


#define GL_CHECK() \
{ \
GLenum glError = glGetError(); \
if (glError != GL_NO_ERROR) {\
LOGE("glGetError() = %i (0x%.8x) in filename = %s, line  = %i\n", glError, glError, __FILE__ , __LINE__); \
} \
}

char* ReadFile2String(const char *path, unsigned long &strLen);
class DX11IMAGEFILTER_EXPORTS_CLASS OpenGLUtil
{
public:
    static void releaseFBO(GLuint tex, GLuint fbo);
    static void readFBO(GLuint tex, void *data, int width, int height);
    static void createToTexture(GLuint &texID, int nWidth, int nHeight, unsigned char*data= NULL,GLenum glFormat = GL_RGBA , bool useMipmap = false);
	static void createToCubeMap(GLuint &texCubeID, int nWidth, int nHeight, GLenum glInternalFormat = GL_RGBA, GLenum glFormat = GL_RGBA,  bool useMipmap = false);

	static void createVAO(GLuint &VAO, GLuint &VBO, float *pVertices,int nVerts, int *pArrange, int nArrange, int nStep);

    static GLuint CreateTextureRGBAF( int width, int height, GLenum glFormat = GL_RGBA16F, GLfloat* pData = NULL);
    static GLuint CreateTextureRGBAF2(int width, int height, GLfloat* pData,GLenum glInternalFormat, GLenum glFormat );

    static void loadFboStatus(const GLint& originalFBO, const GLint* originalViewport);
    static void saveFboStatus(GLint& originalFBO, GLint* originalViewport);
    static GLuint CreatePorgramForText(const char * vs_text, const char * fs_text);

    static GLuint CreatePorgramForFile(const char * vs_file, const char * fs_file);

    static GLuint LoadShaderSource(GLenum shaderType, const char* pSource);

	static int GetOpenGLVersion();

};

#endif // OPENGLUTIL_H
