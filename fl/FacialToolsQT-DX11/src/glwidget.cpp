#include "glwidget.h"
#include <QDebug>
#include "Toolbox/GL/openglutil.h"
#include "Toolbox/GL/FilterBase.h"
#include <QTimer>
#include <gl/wglew.h>

static const GLfloat kVertexCoordindate[] = {-1.0f,-1.0f,1.0f,-1.0f,-1.0f,1.0f,1.0f,1.0f};

//QOpenGLWidget的render 坐标系似乎和移动端不同 需要上下翻转!!!
static const GLfloat kTextureCoordinate[] = {0.0f,1.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f};

static const char kBaseRenderVertexShader[] = R"(
    //#version 300 es
	//layout(location = 0) in vec2 vertexCoordinate;
	//layout(location = 1) in vec2 textureCoordinate;
 //   out vec2 inputTextureCoordinate; 
    attribute vec2 vertexCoordinate;
    attribute vec2 textureCoordinate;
    varying vec2 inputTextureCoordinate;
    void main(){
        inputTextureCoordinate = textureCoordinate;
        gl_Position = vec4(vertexCoordinate.xy, 0, 1);
    }
)";

static const char kBaseRenderFragmentShader[] = R"(
    //#version 300 es
    uniform sampler2D inputTexture;
    //in vec2 inputTextureCoordinate;
    varying vec2 inputTextureCoordinate;
    void main(){
        vec4 color = texture2D(inputTexture, inputTextureCoordinate);
        gl_FragColor = color;
        //out_FragColor = color;
    }
)";

GLWidget::GLWidget(QWidget *parent)
        :QWidget(parent),
         currentTexture(0)
{
	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(render()));
	timer->start(30);
}

GLWidget::~GLWidget()
{

}


BOOL GLWidget::SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR pixelDesc;

	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;

	pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_STEREO_DONTCARE;

	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 32;
	pixelDesc.cRedBits = 8;
	pixelDesc.cRedShift = 16;
	pixelDesc.cGreenBits = 8;
	pixelDesc.cGreenShift = 8;
	pixelDesc.cBlueBits = 8;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 64;
	pixelDesc.cAccumRedBits = 16;
	pixelDesc.cAccumGreenBits = 16;
	pixelDesc.cAccumBlueBits = 16;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 32;
	pixelDesc.cStencilBits = 8;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;

	m_GLPixelIndex = ChoosePixelFormat(hDC, &pixelDesc);
	if (m_GLPixelIndex == 0) // Choose default
	{
		m_GLPixelIndex = 1;
		if (DescribePixelFormat(hDC, m_GLPixelIndex,
			sizeof(PIXELFORMATDESCRIPTOR), &pixelDesc) == 0)
			return FALSE;
	}

	if (!SetPixelFormat(hDC, m_GLPixelIndex, &pixelDesc))
		return FALSE;

	return TRUE;
}

BOOL GLWidget::CreateViewGLContext(HDC hDC)
{
	HWND m_hWnd = (HWND)winId();
	m_hDC = ::GetDC(m_hWnd);
	SetWindowPixelFormat(m_hDC);

	HGLRC tempContext = wglCreateContext(m_hDC);
	wglMakeCurrent(m_hDC, tempContext);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		return FALSE;
	}

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,//主版本3
		WGL_CONTEXT_MINOR_VERSION_ARB, 5,//次版本3
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,//要求返回兼容模式环境,如果不指定或指定为WGL_CONTEXT_CORE_PROFILE_BIT_ARB会返回只包含核心功能的环境
		0
	};

	if (wglewIsSupported("WGL_ARB_create_context") == 1)
	{
		m_hGLContext = wglCreateContextAttribsARB(m_hDC, 0, attribs);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tempContext);
		wglMakeCurrent(m_hDC, m_hGLContext);
	}
	else
	{       //It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		m_hGLContext = tempContext;
	}

	//Checking GL version
	const GLubyte *GLVersionString = glGetString(GL_VERSION);

	//Or better yet, use the GL3 way to get the version number
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	if (!m_hGLContext) return false;

	glewInit();
	if (glewIsSupported("GL_VERSION_4_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		//exit(1);
	}
	return TRUE;
}


void GLWidget::initializeGL()
{
	setAttribute(Qt::WA_UpdatesDisabled, true);
	//setAttribute(Qt::WA_TranslucentBackground, true);
	if (CreateViewGLContext(m_hDC) == FALSE)
		return ;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&m_originalFramebuffer); //来查询后台FBO对象，保存下来；当你结束你的FBO使用，绘制正常渲染时，就要
    glGetIntegerv(GL_VIEWPORT, m_originalViewport);

}

void GLWidget::resizeEvent(QResizeEvent *event)
{
	glViewport(0, 0, width(), height());
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	//qDebug() << "resizeGL:";
	GLint fbo, viewport0[4];
	OpenGLUtil::saveFboStatus(m_originalFramebuffer, m_originalViewport);
	m_originalViewport[0] = 0;
	m_originalViewport[1] = 0;
	m_originalViewport[2] = width();
	m_originalViewport[3] = height();
}

void GLWidget::render()
{
	GL_CHECK();
	m_glWidget->Render();
	GL_CHECK();
	setInputTexture(m_glWidget->getRenderTex(), m_glWidget->getRenderW(), m_glWidget->getRenderH());

	if (currentTexture == 0) {
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(m_originalViewport[0], m_originalViewport[1], m_originalViewport[2], m_originalViewport[3]);
	float scale = static_cast<float>(currentWidth) / static_cast<float>(currentHeight);
	float screenScale = static_cast<float>(m_originalViewport[3]) / static_cast<float>(m_originalViewport[2]);
	scale *= screenScale;
	GLfloat vertexCoordinates[8];
	::memcpy(vertexCoordinates, kVertexCoordindate, sizeof(GLfloat) * 8);
	vertexCoordinates[0] *= scale;
	vertexCoordinates[4] *= scale;
	vertexCoordinates[2] *= scale;
	vertexCoordinates[6] *= scale;

	for (int i = 0; i < 4; i++)
	{
		vertexCoordinates[i * 2 + 0] *= m_scale;
		vertexCoordinates[i * 2 + 1] *= m_scale;
		vertexCoordinates[i * 2 + 0] += m_trans.x;
		vertexCoordinates[i * 2 + 1] += m_trans.y;
	}

	m_3dScene_Scale = 0.0f;

	// qDebug() << m_originalViewport[0] <<"\n";
	// glFlush();
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	//program.bind();

	//program.enableAttributeArray(vertexCoordinateIdx);
	//program.setAttributeArray(vertexCoordinateIdx, vertexCoordinates, 2);
	//program.enableAttributeArray(textureCoordinateIdx);
	//program.setAttributeArray(textureCoordinateIdx, kTextureCoordinate, 2);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, currentTexture);
	//program.setUniformValue(inputTextureIdx, 1);

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//glFlush();
	//program.disableAttributeArray(vertexCoordinateIdx);
	//program.disableAttributeArray(textureCoordinateIdx);
	//program.release();
	static FilterBase _filter;
	_filter.FilterToTexture(currentTexture, m_glWidget->getRenderW(), m_glWidget->getRenderH(), (float*)vertexCoordinates, (float*)kTextureCoordinate);
	SwapBuffers(m_hDC);
}

void GLWidget::initShaders()
{
//     if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, kBaseRenderVertexShader)) {
//         qDebug() << __FILE__ << __FUNCTION__ << "add vertex shader failed" << endl;
//         return;
//     }
// 
//     if (!program.addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, kBaseRenderFragmentShader)) {
//         qDebug() << __FILE__ << __FUNCTION__ << "add fragment shader failed" << endl;
//         return;
//     }
// 
//     program.bindAttributeLocation("vertexCoordinate", 0);
//     program.bindAttributeLocation("textureCoordinate", 1);
//     program.link();
//     program.bind();
// 
//     vertexCoordinateIdx = program.attributeLocation("vertexCoordinate");
//     textureCoordinateIdx = program.attributeLocation("textureCoordinate");
//     inputTextureIdx = program.uniformLocation("inputTexture");
}


void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    update();
}

void GLWidget::setInputTexture(GLuint tex, int width, int height)
{
    currentTexture = tex;
    currentWidth = width;
    currentHeight = height;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{

    m_isPress = true;
    auto point = event->pos();
    m_PressPoint = Vector2(point.x(), point.y());
    m_transDelta = m_trans;

}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{

     m_isPress = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{

    if (m_isPress)
    {
        auto point = event->pos();
        auto movePoint = Vector2(point.x(), point.y());
        auto trans = (movePoint - m_PressPoint);// *vec2(1.0 / m_nWidth, 1.0 / m_nHeight);
        trans.x /= currentWidth;
        trans.y /=-currentHeight;
        m_trans = trans + m_transDelta;
    }

}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    float fDelta = event->delta();
    if (fDelta < 0.0f) {
        //m_scale = m_scale - 0.1f;
		//m_3dScene_Scale = m_3dScene_Scale + 0.1f;
    }
    else {
        //m_scale = m_scale + 0.1f;
		//m_3dScene_Scale = m_3dScene_Scale - 0.1f;
    }

	m_3dScene_Scale = fDelta < 0.0 ? 0.1f : -0.1f;

	m_glWidget->ZoomInOut(m_3dScene_Scale);
}

void GLWidget::keyPressEvent(QKeyEvent* e) {
	if (e->key() == Qt::Key_Space) {
		emit togglePausePlay();
	}
}
