#ifndef D3D11RENDERWIDGET_H
#define D3D11RENDERWIDGET_H

#include <QWidget>	

#include "FaceDetectorInterface.h"
//#include "CCFaceExpressInterface.h"
#include "common.h"
#include "StickerInterface.h"

#include "Toolbox/DXUtils/DX11Context.h"
#include "Toolbox/DXUtils/DX11Texture.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/RectDraw.h"

#include "FaceGiftEffectPlayer.h"
#include "opencv2/opencv.hpp"

#include "FacialAnimationInterface.h"
#include "EffectCInterface.h"
//#include "INetInterface.h"
#include "IFurnaceInterface.h"
#include "HumanParsingInterface.h"
#include "HandDetectInterface.h"
#include "BodyDetectInterface.h"
#include "SplitScreenDetectInterface.h"
#include "Toolbox/inc.h"
#include "Toolbox/Render/SwapChainRHI.h"
#include "Toolbox/vec2.h"
#include <mutex>
#include <chrono>

enum OpenFileType {
	IMAGE_FILE,
	VIDEO_FILE,
	CAMERA,
};

namespace CC3DImageFilter
{
	class CC3DInterface;
}

class D3d11RenderWidget : public QWidget
{
	Q_OBJECT

public:
	D3d11RenderWidget(QWidget *parent = nullptr);
	~D3d11RenderWidget();
	virtual QPaintEngine *paintEngine() const
	{
		return NULL;
	}

	cc_handle_t getSticker() {
		return m_StickerEffect;
	}
	bool changeCameraFileType();
	bool changeImageFileType(QString path);
	bool changeVideoFileType(QString path);
	bool changeEffectResource(QString path);
	bool changeEffectResourceXML(QString dir);
	bool changeGLBResource(QString path);
	bool getShowKeyPoints() { return m_show_keypoints; }
	bool getShowOrigin() { return m_show_origin; }
	bool getAROrSence() { return m_AR; }
	void execMouseEvent(float Scale,const core::vec2f& Rotate,const core::vec2f& Translate);

	byte_t* m_HairMask;

	void PostEvent(std::shared_ptr<class TaskEvent> Event);
	void ReSize(uint32_t width, uint32_t height);
	void Init();

	void LoadAnimateJson(const std::string& fileName);
public slots:
	void setShowKeyPoints(bool show) { m_show_keypoints = show; }
	void setShowOrigin(bool show) { m_show_origin = show; }
	void setSegment(bool show) { m_show_Segment = show; }
protected:
	virtual void resizeEvent(QResizeEvent *event) override;

	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;


	HRESULT prepareRect();
	HRESULT VertexIndex();
	HRESULT InitDevice();
	void Render();
	void RenderGLB();
	void frameReset();
	void getScale(float& x_s, float& y_s) {
		x_s = 1.0f;
		y_s = 1.0f;
		int w_mh = this->width() * m_nHeight;
		int h_mw = this->height() * m_nWidth;

		if (0 == w_mh || 0 == h_mw) return;
		if (w_mh > h_mw) {
			x_s = float(h_mw) / float(w_mh);
		} else {
			y_s = float(w_mh) / float(h_mw);
		}
	}

	static unsigned int _stdcall RenderThread(void* p);
	unsigned int  InnerThread();

	void RunTask();
	
	void MakeMouseEvent(float Scale, const core::vec2f& Rotate,const core::vec2f& Translate);

private:
	std::shared_ptr<SwapChainRHI> m_SwapChain;
	std::shared_ptr<class CC3DRasterizerState> m_Rasterizer;
	std::shared_ptr<class CC3DBlendState> m_BlendState;
	std::shared_ptr<class CC3DDepthStencilState> m_DepthStencialState;
	
	cc_handle_t		 m_FaceDetectHandle = NULL;
	//ccFaceExpress_handle_t m_FaceExpressDetect = NULL;
	cc_handle_t		 m_StickerEffect;
	std::shared_ptr<RectDraw> m_rectDraw;

	IFurnaceInterface* segment_net;
	int frame_index = 0;
	int save_width = 0, padded_w = 0, padded_h = 0;
	int save_height = 0;
	int pad_rate = 16;
	int min_pad = 6;

	int hair_out_w = 0; 
	int hair_out_h = 0; 
	int hair_out_c = 0;

	//用于输入检测的灰度图
	//cv::Mat m_matGrey;
	unsigned char *m_pBuffer;
	int m_nChannel;
	int m_nWidth;
	int m_nHeight;

	//背景纹理
	std::shared_ptr<CC3DTextureRHI> m_pBGTexture;

	std::shared_ptr<CC3DTextureRHI> m_TestMat;

	std::shared_ptr<CC3DTextureRHI> m_renderTargetTex;

	//打开文件的类型（图片、视频、摄像头等）
	OpenFileType	 m_file_type;
	cv::Mat			 m_frame;
	//cv::VideoWriter writer;
	cv::VideoCapture m_camera_or_video;
	class VideoCaptureMapper* m_VideoCaptureMapper = nullptr;

	float m_scale = 1.0f;
	Vector2 m_trans = Vector2(0.0, 0.0);
	Vector2 m_transDelta = Vector2(0.0, 0.0);
	Vector2 m_PressPoint;
	bool m_isPress = false;

	//m_camera_or_video pause
	bool m_isPause;
	//show the face key points
	bool m_show_keypoints;
	//show the origin frame
	bool m_show_origin;
	cc_handle_t handleHand = nullptr;
	bool m_showHand = true;
	bool m_showSegHair = false;
	bool m_showSegBody = true; 
	bool m_show_Segment = false;
	bool m_show_BodyDetect = false;
	cc_handle_t handleBodeyDetect = nullptr;
	//is AR or Sence
	bool m_AR; //true is AR, false is sence


	HANDLE m_RenderThread = nullptr;
	std::chrono::high_resolution_clock::time_point m_Start, m_End;
	bool m_Exit = false;
	std::shared_ptr<class TaskLogic> m_TaskLogic;
	std::recursive_mutex m_TaskLock;
	QList<std::shared_ptr<TaskEvent>> m_EventList;
	std::shared_ptr<CC3DImageFilter::CC3DInterface> m_3DInterface;

	QPoint m_LBDownPoint;
	QPoint m_RBDownPoint;

	Vector2 m_Rotate;
	Vector2 m_LBtnDownTranslate;
	Vector2 m_RBtnDownTranslate;
	Vector2 m_Translate;
	bool m_LeftPress = false;
	bool m_RightPress = false;
};

#endif // D3D11RENDERWIDGET_H
