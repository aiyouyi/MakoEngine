#ifndef D3D11RENDERWIDGET_H
#define D3D11RENDERWIDGET_H

#include <QWidget>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>	

#include "FaceDetectorInterface.h"

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
#include "CCHandGestureInterface.h"

enum OpenFileType {
	IMAGE_FILE,
	VIDEO_FILE,
	CAMERA,
};

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
	bool changeEffectResourceXML(QString dir, QString xml);
	bool change3DEffectResource(QString path);
	bool getShowKeyPoints() { return m_show_keypoints; }
	bool getShowOrigin() { return m_show_origin; }
	bool getAROrSence() { return m_AR; }

	unsigned char* m_HairMask;

public slots:
	void setShowKeyPoints(bool show) { m_show_keypoints = show; }
	void setShowOrigin(bool show) { m_show_origin = show; }
	void setSegment(bool show) { m_show_Segment = show; }
protected:
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void paintEvent(QPaintEvent *event) override;

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

private:
	ID3D11Device			 *m_d3dDevice;
	ID3D11DeviceContext		 *m_d3dDevContext;
	IDXGISwapChain			 *m_swapChain;
	ID3D11Texture2D			 *m_depthStencilBuffer;
	ID3D11DepthStencilView	 *m_depthStencilView;
	ID3D11RenderTargetView	 *m_renderTargetView;
	

	cc_handle_t		 m_StickerEffect;
	RectDraw		 m_rectDraw;

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
	DX11Texture *m_pBGTexture;

	DX11Texture *m_TestMat;

	DX11Texture *m_renderTargetTex;

	//打开文件的类型（图片、视频、摄像头等）
	OpenFileType	 m_file_type;
	cv::Mat			 m_frame;
	cv::VideoCapture m_camera_or_video;

	float m_scale = 1.0f;
	vec2 m_trans = vec2(0.0, 0.0);
	vec2 m_transDelta = vec2(0.0, 0.0);
	vec2 m_PressPoint;
	bool m_isPress = false;

	//m_camera_or_video pause
	bool m_isPause;
	//show the face key points
	bool m_show_keypoints;
	//show the origin frame
	bool m_show_origin;
	bool m_showHand = false;
	bool m_showSegHair = false;
	bool m_showSegBody = false; 
	bool m_show_Segment = false;
	//is AR or Sence
	bool m_AR; //true is AR, false is sence
};

#endif // D3D11RENDERWIDGET_H
