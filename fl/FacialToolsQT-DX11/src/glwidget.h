#ifndef GLWIDGET_H
#define GLWIDGET_H

#include    "BaseDefine/Define.h"
#include    <QEvent>
#include    <QMouseEvent>
#include    "glrenderwiget.h"
#include    "BaseDefine/Vectors.h"


class GLWidget : public QWidget
{
Q_OBJECT
signals:
	void togglePausePlay();

public:
    GLWidget(QWidget *parent = nullptr);
    virtual ~GLWidget() override;
	void initializeGL() ;

	virtual void resizeEvent(QResizeEvent *event) override;
	
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void setInputTexture(GLuint tex, int width, int height);

    void setGLwidget(glRenderWiget *glWidget) { m_glWidget = glWidget; 	QObject::connect(this, &GLWidget::togglePausePlay, m_glWidget, &glRenderWiget::onTogglePausePlay);}

	BOOL SetWindowPixelFormat(HDC hDC);
	// 创建一个GL渲染上下文
	BOOL CreateViewGLContext(HDC hDC);

private slots:
	 void render();

protected:
    void initShaders();

    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* e);

private:
	HGLRC m_hGLContext;
	HDC m_hDC;
	int m_GLPixelIndex;


    int vertexCoordinateIdx;

    int textureCoordinateIdx;
    int inputTextureIdx;

    GLuint currentTexture;
    int currentWidth;
    int currentHeight;

    GLint m_originalFramebuffer;
    GLint m_originalViewport[4];

    glRenderWiget *m_glWidget;


    float m_scale = 1.0f;
	float m_3dScene_Scale = 0.0f;
    Vector2 m_trans = Vector2(0.0, 0.0);
    Vector2 m_transDelta = Vector2(0.0, 0.0);
    Vector2 m_PressPoint;
    bool m_isPress = false;

};

#endif // GLWIDGET_H
