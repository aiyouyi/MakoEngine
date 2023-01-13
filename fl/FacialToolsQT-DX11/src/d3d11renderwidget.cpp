#include <QKeyEvent>
#include "d3d11renderwidget.h"
#include <QDebug>
#include <QPainter>
#include "CImageBeautify.h"
#include "BasicTimer.h"
#include <process.h>
#include <chrono>
#include "EffectKernel/Video/VideoCaptureMapper.hpp"
#include "tasklogic.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/RenderState/RasterizerState.h"
#include "Toolbox/RenderState/BlendState.h"
#include "Toolbox/RenderState/DepthStencilState.h"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "CC3DEngine/CC3DInterface.h"
#include <algorithm>

D3d11RenderWidget::D3d11RenderWidget(QWidget *parent)
	: QWidget(parent),
	segment_net(nullptr),
	m_pBuffer(nullptr),
	m_HairMask(nullptr),
	m_file_type(OpenFileType::IMAGE_FILE),
	m_isPause(false),
	m_show_keypoints(false),
	m_show_origin(true), 
	m_AR(true) {
	//setAttribute(Qt::WA_PaintOnScreen,true);
	setAttribute(Qt::WA_NativeWindow,true);

	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_UpdatesDisabled);

}

D3d11RenderWidget::~D3d11RenderWidget()
{
	SAFE_DELETE(m_VideoCaptureMapper);
	if (m_RenderThread)
	{
		m_Exit = true;
		::WaitForSingleObject(m_RenderThread, 5000);
		::CloseHandle(m_RenderThread);
		m_RenderThread = nullptr;
	}

	if (m_AR) 
	{
		ccEffectRelease(m_StickerEffect);
	} else {
		ccFacialAnimationDestory(m_StickerEffect);
	}

	ccHDUnInit(handleHand);
	ccDestoryGiftInst();

	if (m_HairMask) delete[]m_HairMask;

	ccFDUnInit(m_FaceDetectHandle);
	//ccFEUninit(m_FaceExpressDetect);

	ccUnInitFilter();

	if (handleBodeyDetect)
		ccBDUninit(handleBodeyDetect);

	if (segment_net)
		segment_net->release();
}


void D3d11RenderWidget::frameReset()
{
	m_nWidth = m_frame.cols;
	m_nHeight = m_frame.rows;
	m_renderTargetTex->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_RENDER_TARGET, m_nWidth, m_nHeight);
	//reset segment result
	{
		if (segment_net)
			segment_net->release();
		segment_net = nullptr;
		segment_net = CreateFurnaceInterface();

		if (-1 == segment_net->load_segment_model("../BuiltInResource/BodyMask/mnnmodel_singo_x128.mnn", "../BuiltInResource/BodyMask/mnnmodel_singo.mnn", m_nWidth, m_nHeight, &save_height, &save_width))
		{
			cout << "load_segment_model error!" << endl;
		}
		segment_net->set_frame_interval(60);
    }
	//reset bodyDetect result
	{
	    if (handleBodeyDetect)
			ccBDUninit(handleBodeyDetect);
		handleBodeyDetect = nullptr;
	
		handleBodeyDetect = ccBDInit("../BuiltInResource/BodyDetect/models", 1);
		if (handleBodeyDetect == nullptr) {
			std::cout << "init body detect failed!" << std::endl;
		}

		ccBDActivatePose2D(handleBodeyDetect);

		// 设置人体检测器跳帧数量
		ccBDSetRoiDetectSkipFrames(handleBodeyDetect, 10);

		ccBDActivate2DPoseTracking(handleBodeyDetect);
		ccBDSet2DPoseTrackFrames(handleBodeyDetect, 5);
	}
	//reset hair seg result
	//if (!ccHPActivateHairSeg("../BuiltInResource/SegModels/hair_seg_v1.0.0")) {
	//	std::cout << "Activate HairSeg Faild!" << std::endl;
	//}
	ccHPReset();
	ccHPSetInputSize(m_nWidth, m_nHeight);
	ccHPGetHairMaskOutShape(&hair_out_w, &hair_out_h, &hair_out_c);
	if (m_HairMask) delete[]m_HairMask;
	m_HairMask = new byte_t[hair_out_w*hair_out_h*hair_out_c];
}

unsigned int _stdcall D3d11RenderWidget::RenderThread(void* p)
{
	D3d11RenderWidget* pThis = (D3d11RenderWidget*)p;
	return pThis->InnerThread();
}

unsigned int D3d11RenderWidget::InnerThread()
{
	std::chrono::high_resolution_clock::time_point tStart, tEnd;
	tStart = std::chrono::high_resolution_clock::now();
	while (!m_Exit)
	{
		RunTask();
		tEnd = std::chrono::high_resolution_clock::now();
		float Delta = std::chrono::duration<float, std::milli>(tEnd - tStart).count();	
		if (Delta < (1000.0f / 30.0f))
		{
			//::Sleep(0.1f);
			continue;
		}
		tEnd = std::chrono::high_resolution_clock::now();
		Delta = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
		//std::cout << Delta << "ms" << std::endl;
		tStart = std::chrono::high_resolution_clock::now();

		if (m_3DInterface)
		{
			RenderGLB();
		}
		else
		{
			Render();
		}
	
	}
	return 0;
}

void D3d11RenderWidget::RunTask()
{
	if (!m_TaskLogic)
	{
		return;
	}

	QList<std::shared_ptr<TaskEvent>> EventList;
	{
		std::lock_guard<std::recursive_mutex> Lock(m_TaskLock);
		if (m_EventList.isEmpty())
		{
			return;
		}
		EventList.swap(m_EventList);
	}

	if (EventList.isEmpty())
	{
		return;
	}

	do
	{
		std::shared_ptr<TaskEvent> Event = EventList.front();
		EventList.pop_front();

		m_TaskLogic->Event(Event);

	} while (!EventList.isEmpty());
}

void D3d11RenderWidget::MakeMouseEvent(float Scale, const core::vec2f& Rotate, const core::vec2f& Translate)
{
	std::shared_ptr<QTMouseEvent> Event = std::make_shared<QTMouseEvent>();
	Event->Scale = Scale;
	Event->Rotate = Rotate;
	Event->Translate = Translate;
	PostEvent(Event);
}

void D3d11RenderWidget::ReSize(uint32_t width, uint32_t height)
{
	m_SwapChain->ReSize(width, height);
}

void D3d11RenderWidget::Init()
{
	InitDevice();
	VertexIndex();
	m_rectDraw = std::make_shared<RectDraw>();
	m_rectDraw->init(1, 1, "");
	m_TaskLogic = std::make_shared<TaskLogic>(this);

	m_Exit = false;
	m_RenderThread = (HANDLE)_beginthreadex(nullptr, 0, &D3d11RenderWidget::RenderThread, this, 0, nullptr);
}

void D3d11RenderWidget::LoadAnimateJson(const std::string& fileName)
{
	if (m_3DInterface)
	{
		m_3DInterface->LoadAnimateJson(fileName);
	}
}

bool D3d11RenderWidget::changeCameraFileType()
{
	if (m_file_type == OpenFileType::CAMERA) {
		return true;
	}
	if (m_file_type == OpenFileType::VIDEO_FILE) {
		if (m_camera_or_video.isOpened()) m_camera_or_video.release();
	}
	m_file_type = OpenFileType::CAMERA;

	m_camera_or_video.open(0);
	if (m_camera_or_video.isOpened() == false)
	{
		std::cerr << "Failed opening camera" << std::endl;
		return false;
	}
	m_camera_or_video.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	m_camera_or_video.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	m_camera_or_video >> m_frame;
	cv::flip(m_frame, m_frame, 1);
	frameReset();
	m_isPause = false;
	return true;
}

bool D3d11RenderWidget::changeImageFileType(QString path) 
{
	if (m_camera_or_video.isOpened())
		m_camera_or_video.release();
	m_file_type = OpenFileType::IMAGE_FILE;
	const char *pFileName = path.toUtf8();
	m_frame = cv::imread(pFileName);
	frameReset();
	return true;
}

bool D3d11RenderWidget::changeVideoFileType(QString path)
{
	m_isPause = true;
	if (m_file_type == OpenFileType::CAMERA) {
		if (m_camera_or_video.isOpened()) m_camera_or_video.release();
	}
	m_file_type = OpenFileType::VIDEO_FILE;

	auto utf8 = path.toUtf8();
	std::string fileName;
	fileName.append(utf8.data(), utf8.size());
	
	m_camera_or_video.open(fileName.c_str());
	if (m_camera_or_video.isOpened() == false)
	{
		std::cerr << "Failed opening camera" << std::endl;
		return false;
	}
	m_camera_or_video >> m_frame;
	//  cv::flip(m_frame, m_frame, 1);
	//  cv::transpose(m_frame, m_frame);
	frameReset();
	m_isPause = false;


	//保存视频初始化
	//writer.release();
	//int codec = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
	//writer.open("../Resource/test1.avi", -1, 25,
	//	cv::Size(m_frame.cols, m_frame.rows));

	return true;
}

bool D3d11RenderWidget::changeEffectResource(QString path)
{
	if (m_StickerEffect)
	{
		ccEffectRelease(m_StickerEffect);
		ccDestoryGiftEffect();
		m_StickerEffect = nullptr;
	}

	m_3DInterface.reset();

	m_AR = true;
	m_StickerEffect = ccEffectCreate();
	const char* Rpath = "../BuiltInResource";
	ccEffectSetPath(m_StickerEffect, Rpath);
	ccEffectSetEffectZipAsyn(m_StickerEffect, path.toUtf8(), "test.xml", nullptr, nullptr);


	cv::Mat avatar = cv::imread("./avatar.png",-1);
	cv::cvtColor(avatar,avatar, cv::COLOR_BGRA2RGBA);
	ccEffectSetMask(m_StickerEffect, avatar.ptr(), avatar.cols,avatar.rows,UNKNOW_EFFECT,ANCHOR_IMAGE);

	QImage avatarID(620, 159, QImage::Format_ARGB32);
	avatarID.fill(QColor(0, 0, 0, 0));
	QString m_fromNick = QString::fromLocal8Bit("鸽子:夜晚的风");
	QString imageText = m_fromNick.mid(0, 8);
	imageText += m_fromNick.length() > 8 ? "..." : "";
	QFont font;
	font.setPixelSize(80);
	font.setFamily("Microsoft YaHei");
	font.setBold(true);
	QFontMetrics fm(font);
	int charWidth = fm.width(imageText);
	charWidth = fm.boundingRect(imageText).width();
	QPainter painter(&avatarID);
	QPen pen = painter.pen();
	pen.setColor(QColor(231, 0, 130));//RGB->BGR
	painter.setPen(pen);
	painter.setFont(font);
	painter.drawText(avatarID.rect(), Qt::AlignCenter, imageText);
	
	ccEffectSetMask(m_StickerEffect, avatarID.bits(), avatarID.width(), avatarID.height(), UNKNOW_EFFECT, ANCHOR_ID);

 	//ccSetGiftResourcePath(Rpath);
// 	ccStartGiftEffect(path.toLocal8Bit(), "test.xml", nullptr);
	return true;
}

bool D3d11RenderWidget::changeEffectResourceXML(QString dir)
{
	if (m_StickerEffect)
	{
		ccEffectRelease(m_StickerEffect);
		ccDestoryGiftEffect();
		m_StickerEffect = nullptr;
	}

	m_3DInterface.reset();

	m_AR = true;
	m_StickerEffect = ccEffectCreate();
	const char* Rpath ="../BuiltInResource";
	ccEffectSetPath(m_StickerEffect, Rpath);
	ccEffectAddEffectFromXML(m_StickerEffect, dir.toLocal8Bit(), "test.xml");

	cv::Mat avatar = cv::imread("./avatar.png", -1);
	cv::cvtColor(avatar, avatar, cv::COLOR_BGRA2RGBA);
	ccEffectSetMask(m_StickerEffect, avatar.ptr(), avatar.cols, avatar.rows, UNKNOW_EFFECT, ANCHOR_IMAGE);

	QImage avatarID(620, 159, QImage::Format_ARGB32);
	avatarID.fill(QColor(0, 0, 0, 0));
	QString m_fromNick = QString::fromLocal8Bit("鸽子:夜晚的风");
	QString imageText = m_fromNick.mid(0, 8);
	imageText += m_fromNick.length() > 8 ? "..." : "";
	QFont font;
	font.setPixelSize(80);
	font.setFamily("Microsoft YaHei");
	font.setBold(true);
	QFontMetrics fm(font);
	int charWidth = fm.width(imageText);
	charWidth = fm.boundingRect(imageText).width();
	QPainter painter(&avatarID);
	QPen pen = painter.pen();
	pen.setColor(QColor(231, 0, 130));//RGB->BGR
	painter.setPen(pen);
	painter.setFont(font);
	painter.drawText(avatarID.rect(), Qt::AlignCenter, imageText);

	ccEffectSetMask(m_StickerEffect, avatarID.bits(), avatarID.width(), avatarID.height(), UNKNOW_EFFECT, ANCHOR_ID);
	//std::string path = dir.toLocal8Bit() + "/temp";
	//ccWriteAllEffect(m_StickerEffect, path.c_str());
	return true;
}

bool D3d11RenderWidget::changeGLBResource(QString path)
{
	if (m_StickerEffect)
	{
		ccEffectRelease(m_StickerEffect);
		ccDestoryGiftEffect();
		m_StickerEffect = nullptr;
	}
	m_3DInterface = std::make_shared<CC3DImageFilter::CC3DInterface>();
	m_3DInterface->SetResourcePath("../BuiltInResource");
	m_3DInterface->SetHDRPath("../BuiltInResource/HDR/venice_sunset_1k.hdr");
	m_3DInterface->LoadModel(path.toUtf8().data());
	
	m_renderTargetTex->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_RENDER_TARGET, m_nWidth, m_nHeight);

	return true;
}


void D3d11RenderWidget::execMouseEvent(float Scale, const core::vec2f& Rotate, const core::vec2f& Translate)
{
	if (m_3DInterface)
	{
		m_3DInterface->SetScale(Scale);
		m_3DInterface->SetRotate(Rotate.x, Rotate.y);
		m_3DInterface->SetTranslate(Translate.x, Translate.y);
	}
}

void D3d11RenderWidget::PostEvent(std::shared_ptr<TaskEvent> event)
{
	std::lock_guard<std::recursive_mutex> Lock(m_TaskLock);
	m_EventList.append(event);
}

void D3d11RenderWidget::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);

	std::shared_ptr<ReSizeEvent> Task = std::make_shared<ReSizeEvent>();
	Task->width = width();
	Task->height = height();
	PostEvent(Task);
}

void D3d11RenderWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	if(m_file_type != OpenFileType::IMAGE_FILE)
		m_isPause = !m_isPause;
}

void D3d11RenderWidget::keyPressEvent(QKeyEvent *event)
{
}

void D3d11RenderWidget::keyReleaseEvent(QKeyEvent *event)
{

}

void D3d11RenderWidget::mousePressEvent(QMouseEvent *event)
{

	m_isPress = true;
	auto point = event->pos();
	m_PressPoint = Vector2(point.x(), point.y());
	m_transDelta = m_trans;
	//ccEffectSetEffectZipAsyn(m_StickerEffect, "resource/jinsehuangguan_AR.zip", "test.xml", nullptr, nullptr);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		m_LBDownPoint = event->pos();
		m_LBtnDownTranslate = m_Rotate;
		m_LeftPress = true;
	}
	else if (event->button() == Qt::MouseButton::RightButton)
	{
		m_RBDownPoint = event->pos();
		m_RBtnDownTranslate = m_Translate;
		m_RightPress = true;
	}

}

void D3d11RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_RightPress = false;
	m_LeftPress = false;
	m_isPress = false;
	//ccEffectSetEffectZipAsyn(m_StickerEffect, "resource/Love_AR.zip", "test.xml", nullptr, nullptr);
}

void D3d11RenderWidget::mouseMoveEvent(QMouseEvent *event)
{

	if (m_isPress)
	{
		auto point = event->pos();
		auto movePoint = Vector2(point.x(), point.y());
		auto trans = (movePoint - m_PressPoint);// *vec2(1.0 / m_nWidth, 1.0 / m_nHeight);
		trans.x /= m_nWidth;
		trans.y /=-m_nHeight;
		m_trans = trans + m_transDelta;
	}

	if (m_LeftPress)
	{
		m_Rotate = m_LBtnDownTranslate + Vector2(event->pos().x() - m_LBDownPoint.x(), (event->pos().y() - m_LBDownPoint.y()));
	}
	if (m_RightPress)
	{
		m_Translate = m_RBtnDownTranslate + Vector2(event->pos().x() - m_RBDownPoint.x(), (event->pos().y() - m_RBDownPoint.y()));
	}

	if (m_3DInterface)
	{
		MakeMouseEvent(0.0f, core::vec2f(m_Rotate.x, m_Rotate.y), core::vec2f(m_Translate.x, m_Translate.y));
	}

}

void D3d11RenderWidget::wheelEvent(QWheelEvent *event)
{
	float fDelta = event->delta();
	if (fDelta < 0.0f) {
		m_scale -= 0.1;
	}
	else {
		m_scale += 0.1;
	}

	if (m_3DInterface)
	{
		float TempScale = 0.0;
		if (fDelta < 0.0)
		{
			TempScale = -0.2;
		}
		else
		{
			TempScale = 0.2;
		}

		MakeMouseEvent(TempScale, core::vec2f(m_Rotate.x, m_Rotate.y), core::vec2f(m_Translate.x, m_Translate.y));
	}
}

int g_allFrame =457;
int g_indexFrame = 0;
int g_FileIndex = 8;
HRESULT D3d11RenderWidget::prepareRect()
{
	if (m_file_type == OpenFileType::IMAGE_FILE) {
		m_frame = cv::imread("./model1.jpg");
		char pPath[100];
	//	sprintf(pPath, "./output/%d/org/%d.jpg", g_FileIndex, g_indexFrame);
	//	m_frame = cv::imread(pPath);
		
	}
	else if (m_file_type == OpenFileType::CAMERA) {
		m_camera_or_video.open(0);
		if (m_camera_or_video.isOpened() == false)
		{
			std::cerr << "Failed opening camera" << std::endl;
		}
		m_camera_or_video >> m_frame;
	}
	m_nWidth = m_frame.cols;
	m_nHeight = m_frame.rows;

	int width = m_frame.cols;
	int height = m_frame.rows;

	m_pBGTexture = GetDynamicRHI()->CreateTexture();

	m_renderTargetTex = GetDynamicRHI()->CreateTexture();
	m_renderTargetTex->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_RENDER_TARGET, width, height);

	if (m_AR) {
		m_StickerEffect = ccEffectCreate();

		const char* Rpath = "../BuiltInResource";
		ccEffectSetPath(m_StickerEffect, Rpath);

		ccEffectSetEffectZipSync(m_StickerEffect, "./resource/ALLBeauty/pc_online/FaceBeatiful3.zip", NULL, NULL, NULL);

		ccSetGiftResourcePath(Rpath);
		ccStartGiftEffect("./resource/FaceEffect3DRect__AR.zip", "test.xml", nullptr);

		//ccEffectSetEffectZipSync(m_hStickerEffect, "./resource/MakeUpTest6.zip", "test.xml", NULL, NULL);
		//ccEffectSetEffectZipSync(m_hStickerEffect, "./resource/SPMLS.zip", "test.xml", NULL, NULL);
		//ccEffectSetEffectZipSync(m_StickerEffect, "./resource/MakeUpTest6.zip", "test.xml", NULL, NULL);
		//	ccEffectSetEffectZipSync(m_hStickerEffect, "./resource/SPMLS.zip", "test.xml", NULL, NULL);
		//ccEffectSetEffectZipSync(m_StickerEffect, "./resource/2DTexture.zip", "test.xml", NULL, NULL);
	} else {
		m_StickerEffect = ccFacialAnimationCreate();
		char* Rpath = (char*)"../BuiltInResource";
		ccFacialAnimationSetPath(m_StickerEffect, Rpath);
		ccFacialAnimationLoadScene(m_StickerEffect, "./resource/jiumiNormal.zip");

	}


	//face point init
	m_FaceDetectHandle = ccFDInit("../BuiltInResource/faceModel/");
	//	ccFDSetFilterType(1);
	//	ccFD(0.8f);
	ccFDSetLandmarkFilter(m_FaceDetectHandle, 0.5);
	//face expression control
	ccFDActivateEyeExpression(m_FaceDetectHandle);
	ccActivateStageTwo(m_FaceDetectHandle,"../BuiltInResource/faceModel/");
	ccActivateStageTwoEye(m_FaceDetectHandle,"../BuiltInResource/faceModel/");
	//Face Expression SDK Init
	//m_FaceExpressDetect = ccFEInit("../3rdparty/faceExpressionSDK/model/");
	
	//hair segment
	if (m_showSegHair)
	{
		if (!ccHPActivateHairSeg("../BuiltInResource/HairSegModel/hair_seg_v1.0.0")) {
			std::cout << "Activate HairSeg Faild!" << std::endl;
		}
		ccHPSetInputSize(width, height);
		ccHPGetHairMaskOutShape(&hair_out_w, &hair_out_h, &hair_out_c);
		if (m_HairMask == NULL)
		{
			m_HairMask = new byte_t[hair_out_w*hair_out_h*hair_out_c];
		}
	}
	//Body segment 
	if (m_showSegBody)
	{
		segment_net = CreateFurnaceInterface();
		
		//segment_net->setD3D11Environment(NULL, NULL);
		//segment_net->set_and_calc_segment_params(640, 480, save_height, pad_rate, min_pad, save_width, padded_w, padded_h);

		if (-1 == segment_net->load_segment_model("../BuiltInResource/BodyMask/mnnmodel_singo_x128.mnn","../BuiltInResource/BodyMask/mnnmodel_singo.mnn", width, height, &save_height, &save_width))
		{
			cout << "load_segment_model error!" << endl;
		}

		segment_net->set_frame_interval(60);
	}
	//hand point detect and Gesture Recognition
	if (m_showHand)
	{
		handleHand = ccHDInit("../BuiltInResource/HandModel",0);
		if(!handleHand){
			std::cout << "Activate Hand Model Failed!" << std::endl;
		}
		//开启姿态估计模块
		ccHDActivateHandPose(handleHand);
		//开启双手手势识别模块
		ccHDActivateDoubleGesture(handleHand);
		// 设置为每1帧检测一次
		ccHDSetRoiDetectSkipFrames(handleHand, 4); 
		//设置手势识别判定系数
		ccHDSetDoubleGestureRelaxation(handleHand, 5.0); // 双手手势
		//设置平滑滤波强度 取值越大，抖动越弱，延迟越严重
		ccHDSetPoseSmoothWeight(handleHand, 0.5); // 0~1之间
	}
	if (m_show_BodyDetect)
	{
		handleBodeyDetect = ccBDInit("../BuiltInResource/BodyDetect/models", 1);
		if (handleBodeyDetect == nullptr) {
			std::cout << "init failed!" << std::endl;
			return -1;
		}

		ccBDActivatePose2D(handleBodeyDetect);

		// 设置人体检测器跳帧数量
		ccBDSetRoiDetectSkipFrames(handleBodeyDetect, 10);

		ccBDActivate2DPoseTracking(handleBodeyDetect);
		ccBDSet2DPoseTrackFrames(handleBodeyDetect,5);
	}


	return S_OK;
}


HRESULT D3d11RenderWidget::VertexIndex()
{
	prepareRect();

	HRESULT hr = S_OK;


	m_Rasterizer = GetDynamicRHI()->CreateRasterizerState(CC3DRasterizerState::CT_NONE);
	m_BlendState = GetDynamicRHI()->CreateBlendState(true, false, true, false);
	m_DepthStencialState = GetDynamicRHI()->CreateDefaultStencilState(false, true);

	return hr;
}


int StaticGetSplitType(int SplitType)
{
	switch (SplitType)
	{
	case SSD_SCREEN_TYPE_FULL:
		return 1;
	case SSD_SCREEN_TYPE_TWO_REPLICATE:
		return 2;
	case SSD_SCREEN_TYPE_TWO_MIRROR:
		return 2;
	case SSD_SCREEN_TYPE_THREE_REPLICATE:
		return 3;
	default:
		break;
	}
	return 1;
}

float g_Alpha = 0.0;
void D3d11RenderWidget::Render()
{
	if (!m_isPause) 
	{
		if (m_file_type == OpenFileType::CAMERA) {
			m_camera_or_video >> m_frame;
			cv::flip(m_frame, m_frame, 1);
		}
		if (m_file_type == OpenFileType::VIDEO_FILE) {
			m_camera_or_video >> m_frame;
			if (m_frame.empty()) { 
				m_camera_or_video.set(CV_CAP_PROP_POS_FRAMES, 0);
				return;
			}
			//cv::flip(m_frame, m_frame, 1);
			//cv::transpose(m_frame, m_frame);

			//Sleep(30);

		}
	}

	int width = m_frame.cols;
	int height = m_frame.rows;
	if (width * height == 0)
	{
		return;
	}

    cv::Mat s_frame = m_frame;

	m_pBuffer = (unsigned char *)s_frame.ptr();
	int nChannel = s_frame.channels();
	m_nWidth = s_frame.cols;
	m_nHeight = s_frame.rows;

	ccFDFaceRes_t faceRes;
	faceRes.numFace = 0;

	//ccFaceExpressionResult faceExpressionRes;

	ccBodyRes bodyRes;

	if (1)
	{   //Get Face Point 
		ccFDDetect(m_FaceDetectHandle,m_pBuffer, m_nWidth, m_nHeight, m_nWidth * nChannel, CT_BGR);
		ccFDGetFaceRes(m_FaceDetectHandle,&faceRes);
		//Get FaceExpression result 
		//ccFEDetect(m_FaceExpressDetect, m_pBuffer, faceRes, m_nWidth, m_nHeight, m_nWidth * 3, CT_BGR);
		//ccFEGetRes(m_FaceExpressDetect,&faceExpressionRes);

		//To AR

	}

	int splitType = (int)ccScreenSplitDetect(m_frame.data, m_frame.cols, m_frame.rows, m_frame.cols * 3, SSD_CT_BGR);
	ccEffectSetSplitScreen(m_StickerEffect, splitType, CCEffectType::BODY_TRACK2D_EFFECT);
	//std::cout << "分屏类型：" << splitType << std::endl;

	if (m_showHand)
	{
		double t1 = (double)cv::getTickCount();

		ccHDDetect(handleHand, m_pBuffer, m_nWidth, m_nHeight, m_nWidth * nChannel, HD_CT_BGR);

		double t2 = (double)cv::getTickCount();
		std::cout << "hand detect time:" << (t2 - t1) * 1000 / (cv::getTickFrequency()) << "  ms" << endl;
		ccHandRes resHand = ccHDGetResult(handleHand);

		ccEffectSetHand(m_StickerEffect, &resHand);
	}

	if (m_show_BodyDetect && m_StickerEffect)
	{

		double start = static_cast<double>(cv::getTickCount());
		int typeSplit = StaticGetSplitType(splitType);
		if (!ccBDDetect(handleBodeyDetect, m_frame.ptr(), m_frame.cols, m_frame.rows, m_frame.cols * 3, BD_CT_BGR, typeSplit))
		{
			std::cout << "Detect Body Failed" << std::endl;
		}
		bodyRes = ccBDGetResult(handleBodeyDetect);
		ccEffectSetBodyPoint(m_StickerEffect, &bodyRes);
		
		//double time = ((double)cv::getTickCount() - start) / cv::getTickFrequency();
		//std::cout << "所用时间为：" << time << "秒" << std::endl;
	}

	cvtColor(s_frame, s_frame, cv::COLOR_BGR2RGBA);
	m_pBuffer = (unsigned char *)s_frame.ptr();

	cv::Mat frame_BGRA;
	cvtColor(m_frame, frame_BGRA, cv::COLOR_BGR2BGRA);

	if (m_StickerEffect)
	{
		ccEffectSetBGRA(m_StickerEffect, (unsigned char*)frame_BGRA.ptr());
	}
	
	
	{
		unsigned char *pImageSource = m_pBuffer;

		     //body point
			if (m_show_keypoints&&m_show_BodyDetect) {
				for (int i = 0; i < bodyRes.numBody; ++i) {
					for (int j = 0; j < 16; ++j) {

						ccBodyPoint pt = bodyRes.arrBody[i].keyPointArr[j];
						int x = int(pt.x);
						int y = int(pt.y);
						unsigned char *pTemp = pImageSource + m_nWidth * 4 * y + x * 4;
						for (int dx = -5; dx <= 5; ++dx) {
							for (int dy = -5; dy <= 5; ++dy) {
								unsigned char *pWrite = pTemp + dy * m_nWidth * 4 + dx * 4;
								if (y + dy > 2 && y + dy < m_nHeight - 1 && x + dx > 2 && x + dx < m_nWidth - 2) {
									pWrite[0] = 255;
									pWrite[1] = 0;
									pWrite[2] = 0;
									pWrite[3] = 255;
								}
							}
						}
					}

				}
			}
			if (m_show_keypoints)
			{
				float left = width * 2;
				float right = -1000;
				float top = height * 2;
				float bottom = -1000;

				for (int i = 0; i < faceRes.numFace; ++i) {
					for (int j = 0; j < 130; ++j) {

						ccFDPoint pt = faceRes.arrFace[i].arrShapeDense[j];
						int x = int(pt.x);
						int y = int(pt.y);
						unsigned char* pTemp = pImageSource + m_nWidth * 4 * y + x * 4;
						for (int dx = -2; dx <= 2; ++dx) {
							for (int dy = -2; dy <= 2; ++dy) {
								unsigned char* pWrite = pTemp + dy * m_nWidth * 4 + dx * 4;
								if (y + dy > 2 && y + dy < m_nHeight - 1 && x + dx > 2 && x + dx < m_nWidth - 2) {
									pWrite[0] = 255;
									pWrite[1] = 0;
									pWrite[2] = 0;
									pWrite[3] = 255;
								}
							}
						}



						//ccFDPoint pVerts = faceRes.arrFace[i].arrShapeDense[j];

						//float x1 = pVerts.x;
						//float y1 = pVerts.y;
						//left = (std::min)(left, x1);
						//right = (std::max)(right, x1);
						//top = (std::min)(top, y1);
						//bottom = (std::max)(bottom, y1);



					}

					//std::cout << "Left==>" << left << "        right==>" << right << std::endl;
					//std::cout << "top==>" << top << "        bottom==>" << bottom << std::endl;

				}
			}

			m_pBGTexture->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_NONE, m_nWidth, m_nHeight, pImageSource, m_nWidth * 4);
			m_rectDraw->setShaderTextureView(m_pBGTexture);

	}

	if (m_show_origin && m_StickerEffect)
	{
		if (m_AR) {
			//enable seg body mask
			if (m_showSegBody)
			{
				unsigned char* mask = segment_net->detect_segment_screen(m_frame.cols, m_frame.rows, m_frame.ptr(), BGR888, 1, splitType);
				ccEffectSetMask(m_StickerEffect, mask, save_width, save_height);
			}

			//enable hair mask
			if (m_showSegHair)
			{
				bool ret = ccHPParsing(m_frame.data, m_frame.cols, m_frame.rows, m_frame.cols * 3, 3);
				if (!ret) {
					std::cout << "Seg Hair Mask Error" << endl;
				}

				ccHPGetHairMask(m_HairMask, hair_out_w*hair_out_h*hair_out_c);
				ccEffectSetHairMask(m_StickerEffect, m_HairMask, hair_out_w, hair_out_h);
			}

			if (g_Alpha>1)
			{
				g_Alpha = 0;
			}

			ccEffectSetAlpha(m_StickerEffect, g_Alpha, SMOOTH_EFFECT);
		//	ccEffectSetAlpha(m_StickerEffect, 0.5, FACE_YAMANE_EFFECT);
		//	ccEffectSetAlpha(m_StickerEffect, 0.0, RHYTHM_EFFECT);
			if (ccEffectProcessTexture(m_StickerEffect,RHIResourceCast(m_pBGTexture.get())->GetSRV(), RHIResourceCast(m_renderTargetTex.get())->GetNativeTex(),
				m_pBGTexture->GetWidth(), m_pBGTexture->GetHeight(), &faceRes))
			{
				//ccRenderMatting(m_StickerEffect, m_pBGTexture->getTexShaderView(), m_renderTargetTex->getTex(), m_pBGTexture->width(), m_pBGTexture->height(), 0.3);
				if (m_show_Segment)
				{
					cv::Mat img = cv::imread("../Resource/CropEyeAndEyeBrow/eyeBrow/rouwu.png",-1);
					cvtColor(img, img, cv::COLOR_BGRA2RGBA);
					ccEffectSetMask(m_StickerEffect, img.data, img.cols, img.rows, FACE_EYEBROWSTRICK);

					img = cv::imread("../Resource/CropEyeAndEyeBrow/mouth/maoyan.png",-1);
					cvtColor(img, img, cv::COLOR_BGRA2RGBA);
					ccEffectSetMask(m_StickerEffect, img.data, img.cols, img.rows, FACE_EYESTRICK);

					img = cv::imread("../Resource/CropEyeAndEyeBrow/saihong/naizui.png",-1);
					cvtColor(img, img, cv::COLOR_BGRA2RGBA);
					ccEffectSetMask(m_StickerEffect, img.data, img.cols, img.rows, FACE_BLUSH);

					ccEffectSetColor(m_StickerEffect, 83, 10, 10, 255);
					//ccRenderMattingAfterEffect(m_StickerEffect, m_renderTargetTex->getTex());
				}
				m_rectDraw->setShaderTextureView(m_renderTargetTex);
				
				//if (GetDynamicRHI()->ReadTextureToCpu(m_renderTargetTex, m_pBuffer))
				//{
				//	cv::Mat resultImg =cv::Mat(height, width, CV_8UC4, m_pBuffer);
				//	cv::cvtColor(resultImg, resultImg, cv::COLOR_RGBA2BGR);
				//	cv::imwrite("res1111.png", resultImg);
				//}
			}
			else
			{
				m_rectDraw->setShaderTextureView(m_pBGTexture);
				
			}

		} 
		
		else {
			//ccFacialAnimationUpdateAvantarParam(m_StickerEffect, res.arrAlpha);
			//res.numFace = 0;
			if (ccFacialAnimationProcessTextureExt(m_StickerEffect, RHIResourceCast(m_pBGTexture.get())->GetSRV(), RHIResourceCast(m_renderTargetTex.get())->GetNativeTex(),
				m_pBGTexture->GetWidth(), m_pBGTexture->GetHeight(), &faceRes)) {
				m_rectDraw->setShaderTextureView(m_renderTargetTex);
			} else {
				m_rectDraw->setShaderTextureView(m_pBGTexture);
			}
			
		}
	}

	GetDynamicRHI()->SetViewPort(0, 0, this->width(), this->height());
	m_SwapChain->SetRenderTarget();
	m_SwapChain->Clear(0, 0, 0, 1);

	float x_s = 1;
	float y_s = 1;
	getScale(x_s, y_s);
	//auto blender = DXUtils::CreateBlendState();
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(blender, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetBlendState(m_BlendState, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetRasterizerState(m_Rasterizer);
	GetDynamicRHI()->SetDepthStencilState(m_DepthStencialState);
	m_rectDraw->render(m_trans, Vector2(m_scale* x_s, m_scale*y_s), 0, this->width(), this->height());

	m_SwapChain->Present();
}

void D3d11RenderWidget::RenderGLB()
{
	m_3DInterface->Render(this->width(), this->height(),m_renderTargetTex);
	m_rectDraw->setShaderTextureView(m_renderTargetTex);

	GetDynamicRHI()->SetViewPort(0, 0, this->width(), this->height());
	m_SwapChain->SetRenderTarget();
	m_SwapChain->Clear(0, 0, 0, 1);

	float x_s = 1;
	float y_s = 1;
	getScale(x_s, y_s);
	//auto blender = DXUtils::CreateBlendState();
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	//DeviceContextPtr->OMSetBlendState(blender, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetBlendState(m_BlendState, blendFactor, 0xffffffff);
	GetDynamicRHI()->SetRasterizerState(m_Rasterizer);
	GetDynamicRHI()->SetDepthStencilState(m_DepthStencialState);
	m_rectDraw->render(Vector2(0,0), Vector2(1.0 * x_s, 1.0 * y_s), 0, this->width(), this->height());

	m_SwapChain->Present();
}

//--------------------------------------------------------------------------------------  
// Create Direct3D device and swap chain  
//--------------------------------------------------------------------------------------  
HRESULT D3d11RenderWidget::InitDevice()
{
	HRESULT hr = S_OK;

	m_SwapChain = GetDynamicRHI()->CreateSwapChain();
	if (GetDynamicRHI()->CreateDevice(true))
	{
		if (m_SwapChain->CreateSwapChain((void*)winId(), true, 1280, 720))
		{
			return S_OK;
		}
	}

	return E_FAIL;
}