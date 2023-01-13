#include "openglutil.h"
#include <string>
#include <sstream>


void OpenGLUtil::createToTexture(GLuint &texID, int nWidth, int nHeight, unsigned char*data, GLenum glFormat, bool useMipmap)
{
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, nWidth, nHeight, 0, glFormat,
                 GL_UNSIGNED_BYTE, data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	if (useMipmap)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}


    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLUtil::createToCubeMap(GLuint & texCubeID, int nWidth, int nHeight, GLenum glInternalFormat, GLenum glFormat, bool useMipmap)
{
	glGenTextures(1, &texCubeID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texCubeID);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, glInternalFormat, nWidth, nHeight, 0, glFormat, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (useMipmap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void OpenGLUtil::createVAO(GLuint & VAO, GLuint & VBO, float * pVertices, int nVerts, int * pArrange, int nArrange, int nStep)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nStep*nVerts, pVertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(VAO);

	int nSum = 0;
	for (int i =0;i<nArrange;i++)
	{
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, pArrange[i], GL_FLOAT, GL_FALSE, nStep * sizeof(float), (void*)(nSum*sizeof(float)));
		nSum += pArrange[i];

	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



GLuint OpenGLUtil::CreateTextureRGBAF( int width, int height, GLenum glFormat, GLfloat* pData)
{
    GLuint textures = 0;
    glGenTextures(1, &textures);
    glBindTexture(GL_TEXTURE_2D, textures);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, GL_RGBA, GL_FLOAT, pData);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE, pData);
    return textures;

}

GLuint OpenGLUtil::CreateTextureRGBAF2(int width, int height, GLfloat* pData, GLenum glInternalFormat, GLenum glFormat)
{
	GLuint textures = 0;
	glGenTextures(1, &textures);
	glBindTexture(GL_TEXTURE_2D, textures);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glFormat, GL_FLOAT, pData);

	return textures;
}

void OpenGLUtil::releaseFBO(GLuint tex, GLuint fbo)
{
    if (tex != 0) {
        glDeleteTextures(1, &tex);
    }
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
    }
}

void OpenGLUtil::readFBO(GLuint tex, void *data, int width, int height)
{
    GLint lastFBO[1];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, lastFBO);
    GLint lastViewport[4];
    glGetIntegerv(GL_VIEWPORT, lastViewport);

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glViewport(0, 0, width, height);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO[0]);
    glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
}



void OpenGLUtil::loadFboStatus(const GLint& originalFBO, const GLint* originalViewport)
{
    glBindFramebuffer(GL_FRAMEBUFFER,originalFBO);
    glViewport(originalViewport[0],originalViewport[1],originalViewport[2],originalViewport[3]);
}

void OpenGLUtil::saveFboStatus(GLint& originalFBO, GLint* originalViewport)
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFBO); //来查询后台FBO对象，保存下来；当你结束你的FBO使用，绘制正常渲染时，就要
    glGetIntegerv(GL_VIEWPORT, originalViewport);
}

GLuint OpenGLUtil::LoadShaderSource(GLenum shaderType, const char* pSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*)malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("GLUtils::LoadShaderSource: Could not compile shader %d:\n%s",
                        shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    else {
        LOGE("GLUtils::LoadShaderSource: error to gen shader");
    }
    return shader;
}


// 绝对路径
char* ReadFile2String(const char *path, unsigned long &strLen)
{
    if (path == nullptr)
    {
        LOGE("arkernel::IO::ReadFile2String: Path is nullptr !");
        return nullptr;
    }

    FILE *file = fopen(path, "rb");

    if (file == nullptr) {
            LOGE("ReadFile2String: Could not open file %s !", path);
            return nullptr;
        }
    else{
        fseek(file, 0, SEEK_END);
        int len = (int)ftell(file);
        char* str = new char[len + 1];
        if (str == nullptr) {
            return nullptr;
        }
        fseek(file, 0, SEEK_SET);
        fread(str, len, 1, file);
        str[len] = '\0';
        fclose(file);
        strLen = len;
        return str;
    }
    return nullptr;
}



GLuint OpenGLUtil::CreatePorgramForText(const char * vs_text, const char * fs_text)
{
    GLuint vertID = LoadShaderSource(GL_VERTEX_SHADER, vs_text);

    GLuint fragID = LoadShaderSource(GL_FRAGMENT_SHADER, fs_text);
    if (vertID == 0 || fragID == 0)
    {
        glDeleteShader(vertID);
        glDeleteShader(fragID);
        return 0;
    }
    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertID);
        glAttachShader(program, fragID);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*)malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    if (vertID) {
        glDeleteShader(vertID);
        vertID = 0;
    }
    if (fragID) {
        glDeleteShader(fragID);
        fragID = 0;
    }

    return program;

}

GLuint OpenGLUtil::CreatePorgramForFile(const char *vs_file, const char *fs_file)
{
    unsigned long fileSize = 0;
    char *pShaderCodeSrcvs = ReadFile2String(vs_file, fileSize);
    if (pShaderCodeSrcvs == nullptr) {
        LOGE("GLUtils::CreateProgram: Could not Load byte from %s", vs_file);
        return 0;
    }
    char * pShaderCodeSrcfs = ReadFile2String(fs_file, fileSize);
    if (pShaderCodeSrcfs == nullptr) {
        LOGE("GLUtils::CreateProgram: Could not Load byte from %s", fs_file);
        return 0;
    }

    GLuint re =  CreatePorgramForText(pShaderCodeSrcvs,pShaderCodeSrcfs);
    SAFE_DELETE_ARRAY(pShaderCodeSrcfs);
    SAFE_DELETE_ARRAY(pShaderCodeSrcvs);
    return re;

}

int OpenGLUtil::GetOpenGLVersion() {
	const char* versionStr = (const char*)glGetString(GL_VERSION);
	if (versionStr == NULL) {
		return -1;
	}
	std::string strVersion(versionStr);
	std::size_t numPos = strVersion.find_first_of("1234567890");
	if (numPos == std::string::npos) {
		return -1;
	}
	std::string numStr = strVersion.substr(numPos);
	std::size_t found = numStr.find('.');
	while (found != std::string::npos) {
		numStr[found] = ' ';
		found = numStr.find('.', found + 1);
	}

	int major = 0, minor = 0;
	std::istringstream is(numStr.c_str());
	is >> major >> minor;
	int glVersion = major * 10 + minor;
#if defined(PLATFORM_ANDROID)
	//android需要初始化
//	int nSupportBite = 0;
//	if (glVersion >= 30 && (!gl3stubInit() || !IsSupportGlCaffe(&nSupportBite))) {
//		glVersion = 20;
//	}
//	glVersion = 20; // 安卓底层强制降为2.0，禁用抗锯齿。
#endif
	return glVersion;
}