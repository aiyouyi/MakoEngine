#include "FilterBase.h"


static const GLfloat g_kVertexCoordindate[] = {-1.0f,-1.0f,1.0f,-1.0f,-1.0f,1.0f,1.0f,1.0f};

//QOpenGLWidget的render 坐标系似乎和移动端不同 需要上下翻转!!!
static const GLfloat g_kTextureCoordinate[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};

static const char g_kBaseRenderVertexShader[] = R"(
    attribute vec2 vertexCoordinate;
    attribute vec2 textureCoordinate;
    varying vec2 inputTextureCoordinate;
    void main(){
        gl_Position = vec4(vertexCoordinate.xy, 0.0, 1.0);
        inputTextureCoordinate = textureCoordinate;
    }
)";

static const char g_kBaseRenderFragmentShader[] = R"(
    #ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
    #else
    precision mediump float;
    #endif
    #else
    #define highp
    #define mediump
    #define lowp
    #endif
    uniform sampler2D inputTexture;
    varying vec2 inputTextureCoordinate;
    void main(){
        vec4 color = texture2D(inputTexture, inputTextureCoordinate);
        gl_FragColor = color;//vec4(1.0,0.0,0.0,1.0);
    }
)";

static const char g_kBaseRenderFragmentGrayShader[] = R"(
    #ifdef GL_ES
    #ifdef GL_FRAGMENT_PRECISION_HIGH
    precision highp float;
    #else
    precision mediump float;
    #endif
    #else
    #define highp
    #define mediump
    #define lowp
    #endif
    uniform sampler2D inputTexture;
    varying vec2 inputTextureCoordinate;
    void main(){
        vec4 color = texture2D(inputTexture, inputTextureCoordinate);
        float gray = dot(color.rgb,vec3(0.299,0.587,0.114));
        gl_FragColor = vec4(gray,gray,gray,gray);
    }
)";



FilterBase::FilterBase()
{
    m_program = NULL;
    m_programGray = NULL;
}

FilterBase::~FilterBase()
{
    SAFE_DELETE(m_program);
    SAFE_DELETE(m_programGray);
}

void FilterBase::FilterToTexture(GLuint inputTex, int nWidth, int nHeight)
{
    if(m_program==NULL)
    {
        m_program = new CCProgram();
        m_program->CreatePorgramForText(g_kBaseRenderVertexShader,g_kBaseRenderFragmentShader);
    }

    GL_CHECK();
    glViewport(0,0,nWidth,nHeight);
    //glClearColor(1.0,0.0,1.0,1.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    m_program->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    m_program->SetUniform1i("inputTexture",0);
    m_program->SetVertexAttribPointer("vertexCoordinate", 2, GL_FLOAT, false, 0, g_kVertexCoordindate);
    m_program->SetVertexAttribPointer("textureCoordinate", 2, GL_FLOAT, false, 0, g_kTextureCoordinate);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_program->DisableVertexAttribPointer("vertexCoordinate");
    m_program->DisableVertexAttribPointer("textureCoordinate");
}

void FilterBase::FilterToTexture(GLuint inputTex, int nWidth, int nHeight,float *pVertex,float *pTexCoord)
{
	if (m_program == NULL)
	{
		m_program = new CCProgram();
		m_program->CreatePorgramForText(g_kBaseRenderVertexShader, g_kBaseRenderFragmentShader);
	}

	GL_CHECK();
//	glViewport(0, 0, nWidth, nHeight);
	m_program->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTex);
	m_program->SetUniform1i("inputTexture", 0);
	m_program->SetVertexAttribPointer("vertexCoordinate", 2, GL_FLOAT, false, 0, pVertex);
	m_program->SetVertexAttribPointer("textureCoordinate", 2, GL_FLOAT, false, 0, pTexCoord);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_program->DisableVertexAttribPointer("vertexCoordinate");
	m_program->DisableVertexAttribPointer("textureCoordinate");
}

void FilterBase::FilterToGray(GLuint inputTex, int nWidth, int nHeight)
{
    if(m_programGray==NULL)
    {
        m_programGray = new CCProgram();
        m_programGray->CreatePorgramForText(g_kBaseRenderVertexShader,g_kBaseRenderFragmentGrayShader);
    }
    glViewport(0,0,nWidth,nHeight);
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    m_programGray->Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTex);
    m_programGray->SetUniform1i("inputTexture",0);
    m_programGray->SetVertexAttribPointer("vertexCoordinate", 2, GL_FLOAT, false, 0, g_kVertexCoordindate);
    m_programGray->SetVertexAttribPointer("textureCoordinate", 2, GL_FLOAT, false, 0, g_kTextureCoordinate);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_programGray->DisableVertexAttribPointer("vertexCoordinate");
    m_programGray->DisableVertexAttribPointer("textureCoordinate");
}


