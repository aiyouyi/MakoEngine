#ifndef GLRENDERWIGET_H
#define GLRENDERWIGET_H

#include <QWidget>
#include "Toolbox/GL/openglutil.h"
#include "EffectKernel/CCEffectInterfaceGL.h"
#include "FaceDetector.h"
#include "CC3DEngine/FaceExpressDrive/CC3DExpressDrive.h"
#include "EffectKernel/Video/VideoCaptureMapper.hpp"


class glRenderWiget : public QWidget
{
    Q_OBJECT
public:
    explicit glRenderWiget(QWidget *parent = nullptr);
    ~glRenderWiget();

	bool changeCamera();
    bool changeImageFileType(QString path);
    bool changeVideoFileType(QString path);
	//bool changeAudioFileType(QString path);
    bool changeEffectResource(QString path);

    bool getShowKeyPoints() { return m_show_keypoints; }
    bool getShowOrigin() { return m_show_origin; }
    GLuint getRenderTex()
    {
		return m_showTexture;
    }
    int getRenderW() { return m_nWidth; }
    int getRenderH() { return m_nHeight; }
	void ZoomInOut(float scale);
    void Render();

    CCEffectInterfaceGL *m_InterFace;

	FaceDetector *m_FaceDet;
	CC3DExpressDrive* m_ExpressDrive = NULL;
	cv::VideoCapture cap;
	cv::Mat frame;
	cv::Mat cv_img;

	VideoCaptureMapper* m_VideoCapture;
	
	bool flag = true;
	bool bVideo = true;

public slots:
    void setShowKeyPoints(bool show) { m_show_keypoints = show; }
    void setShowOrigin(bool show) { m_show_origin = show; }
	void onTogglePausePlay();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

private:

    unsigned char *m_pBuffer;
    int m_nChannel;
    int m_nWidth;
    int m_nHeight;

    GLuint m_pBGTexture;

    GLuint m_renderTargetTex;

	GLuint m_showTexture;
    bool m_show_keypoints;
    //show the origin frame
    bool m_show_origin;

	//音频
	float* frame_wav_buffer = nullptr;
	bool wavFileLoaded = false;
	int kWindowLength = 480;
	std::chrono::time_point<std::chrono::system_clock> lastTimeStamp = std::chrono::system_clock::now();
};

#endif // GLRENDERWIGET_H
