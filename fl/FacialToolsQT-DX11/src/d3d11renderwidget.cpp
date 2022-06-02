#include <QKeyEvent>
#include "d3d11renderwidget.h"
#include <QDebug>
#include "CImageBeautify.h"
#include "BasicTimer.h"
//fps��ص�ȫ�ֱ���
double countsPerSecond=0.0;
__int64 CounterStart=0;

int frameCount=0;
int fps=0;

__int64 frameTimeOld=0;
double frameTime;

void startFPStimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double getTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

//��ȡÿ֡���ʱ��
double getFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - frameTimeOld;
	frameTimeOld = currentTime.QuadPart;

	if (tickCount < 0.0f)
		tickCount = 0.0f;

	return float(tickCount) / countsPerSecond;
}

D3d11RenderWidget::D3d11RenderWidget(QWidget *parent)
	: QWidget(parent),
	m_d3dDevice(0),       //��ʼ������ָ��,Ҳ����NULL��nullptr
	m_d3dDevContext(0),
	m_swapChain(0),
	m_depthStencilBuffer(0),
	m_depthStencilView(0),
	m_renderTargetView(0),
	segment_net(nullptr),
	m_pBuffer(nullptr),
	m_pBGTexture(nullptr),
	m_renderTargetTex(nullptr),
	m_HairMask(nullptr),
	m_file_type(OpenFileType::IMAGE_FILE),
	m_isPause(false),
	m_show_keypoints(true),
	m_show_origin(true), 
	m_AR(true) {
	setAttribute(Qt::WA_PaintOnScreen,true);
	setAttribute(Qt::WA_NativeWindow,true);

	InitDevice();
	VertexIndex();
}

D3d11RenderWidget::~D3d11RenderWidget()
{
	if (m_AR) {
		ccEffectRelease(m_StickerEffect);
	} else {
		ccFacialAnimationDestory(m_StickerEffect);
	}

	ccDestoryGiftInst();

	if (m_HairMask) delete[]m_HairMask;
}

void D3d11RenderWidget::paintEvent(QPaintEvent *event)
{
	//����fps
	frameCount++;
	if(getTime() > 1.0f)
	{
		fps=frameCount;
		frameCount=0;
		startFPStimer();
		//���ø����ڱ�����ʾfpsֵ
		//parentWidget()->setWindowTitle("FPS: "+QString::number(fps));
	}
	frameTime=getFrameTime();
	
	Render();
	update();
}

void D3d11RenderWidget::frameReset() {
	m_nWidth = m_frame.cols;
	m_nHeight = m_frame.rows;
	m_renderTargetTex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, m_nWidth, m_nHeight);
	if (segment_net) delete segment_net;
	segment_net = CreateFurnaceInterface();
	//segment_net->setD3D11Environment(NULL, NULL);
	//segment_net->set_and_calc_segment_params(640, 480, save_height, pad_rate, min_pad, save_width, padded_w, padded_h);

	if (-1 == segment_net->load_segment_model("../3rdparty/BodyMask/mnnmodel_singo_x128.mnn", "../3rdparty/BodyMask/mnnmodel_singo.mnn", m_nWidth, m_nHeight, &save_height, &save_width, 2))
	{
		cout << "load_segment_model error!" << endl;
	}

	//reset hair seg result
	
	//if (!ccHPActivateHairSeg("../BuiltInResource/SegModels/hair_seg_v1.0.0")) {
	//	std::cout << "Activate HairSeg Faild!" << std::endl;
	//}
	ccHPReset();
	ccHPSetInputSize(m_nWidth, m_nHeight);
	ccHPGetHairMaskOutShape(&hair_out_w, &hair_out_h, &hair_out_c);
	if (m_HairMask) delete[]m_HairMask;
	m_HairMask = new byte[hair_out_w*hair_out_h*hair_out_c];
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
	const char *pFileName = path.toLocal8Bit();
	m_frame = cv::imread(pFileName);
	frameReset();
	return true;
}

bool D3d11RenderWidget::changeVideoFileType(QString path)
{
	if (m_file_type == OpenFileType::CAMERA) {
		if (m_camera_or_video.isOpened()) m_camera_or_video.release();
	}
	m_file_type = OpenFileType::VIDEO_FILE;
	const char *pFileName = path.toLocal8Bit();
	m_camera_or_video.open(pFileName);
	if (m_camera_or_video.isOpened() == false)
	{
		std::cerr << "Failed opening camera" << std::endl;
		return false;
	}
	m_camera_or_video >> m_frame;
//	cv::flip(m_frame, m_frame, 1);
//	cv::transpose(m_frame, m_frame);
	frameReset();
	m_isPause = false;
	return true;
}

bool D3d11RenderWidget::changeEffectResource(QString path)
{
	if (m_AR) {
		ccEffectRelease(m_StickerEffect);
		ccDestoryGiftEffect();
	} else {
		ccFacialAnimationDestory(m_StickerEffect);
		
	}

	m_AR = true;
	m_StickerEffect = ccEffectCreate();
	char* Rpath = (char*)"../BuiltInResource";
	ccEffectSetPath(m_StickerEffect, Rpath);
	ccEffectSetEffectZipAsyn(m_StickerEffect, path.toUtf8(), "test.xml", nullptr, nullptr);

// 	ccSetGiftResourcePath(Rpath);
// 	ccStartGiftEffect(path.toLocal8Bit(), "test.xml", nullptr);
	return true;
}

bool D3d11RenderWidget::changeEffectResourceXML(QString dir, QString xml)
{
	if (m_AR) {
		ccEffectRelease(m_StickerEffect);
		ccDestoryGiftEffect();
	}
	else {
		ccFacialAnimationDestory(m_StickerEffect);

	}

	m_AR = true;
	m_StickerEffect = ccEffectCreate();
	char* Rpath =(char*)"../BuiltInResource";
	ccEffectSetPath(m_StickerEffect, Rpath);
	ccEffectAddEffectFromXML(m_StickerEffect, dir.toLocal8Bit(), "test.xml");

	//std::string path = dir.toLocal8Bit() + "/temp";
	//ccWriteAllEffect(m_StickerEffect, path.c_str());
	return true;
}

bool D3d11RenderWidget::change3DEffectResource(QString path)
{
	if (m_AR) {
		ccEffectRelease(m_StickerEffect);
	}
	else {
		ccFacialAnimationDestory(m_StickerEffect);
	}
	m_AR = false;
	m_StickerEffect = ccFacialAnimationCreate();
	char * Rpath = (char*)"../BuiltInResource";
	ccFacialAnimationSetPath(m_StickerEffect, Rpath);
	ccFacialAnimationLoadScene(m_StickerEffect, path.toLocal8Bit());

	return true;
}

void D3d11RenderWidget::resizeEvent(QResizeEvent *event)
{
	//�����ڳߴ�仯ʱ��������������֡���桢��Ȼ��桢ģ�建�桢�ӿ�
	if(m_renderTargetView) m_renderTargetView->Release();
	if(m_depthStencilView) m_depthStencilView->Release();
	if(m_depthStencilBuffer) m_depthStencilBuffer->Release();

	HRESULT hr;
	m_swapChain->ResizeBuffers(1,width(),height(),DXGI_FORMAT_R8G8B8A8_UNORM,0);
	//Create our BackBuffer
	ID3D11Texture2D* backBuffer;
	m_swapChain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&backBuffer);

	//Create our Render Target
	hr=m_d3dDevice->CreateRenderTargetView(backBuffer,NULL,&m_renderTargetView);
	if(backBuffer) backBuffer->Release();

	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width=width();
	depthStencilDesc.Height=height();
	depthStencilDesc.MipLevels=1;
	depthStencilDesc.ArraySize=1;
	depthStencilDesc.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count=1;
	depthStencilDesc.SampleDesc.Quality=0;
	depthStencilDesc.Usage=D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags=D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags=0;
	depthStencilDesc.MiscFlags=0;

	//Create the Depth/Stencil View
	m_d3dDevice->CreateTexture2D(&depthStencilDesc,NULL,&m_depthStencilBuffer);
	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer,NULL,&m_depthStencilView);

	//Set our Render Target
	m_d3dDevContext->OMSetRenderTargets(1,&m_renderTargetView,m_depthStencilView);

	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport,sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX=0;
	viewport.TopLeftY=0;
	viewport.Width=width();
	viewport.Height=height();
	viewport.MinDepth=0.0f;
	viewport.MaxDepth=1.0f;

	//Set the Viewport
	m_d3dDevContext->RSSetViewports(1,&viewport);
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
	m_PressPoint = vec2(point.x(), point.y());
	m_transDelta = m_trans;
	//ccEffectSetEffectZipAsyn(m_StickerEffect, "resource/jinsehuangguan_AR.zip", "test.xml", nullptr, nullptr);
}

void D3d11RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	//ccEffectSetEffectZipAsyn(m_StickerEffect, "resource/Love_AR.zip", "test.xml", nullptr, nullptr);
}

void D3d11RenderWidget::mouseMoveEvent(QMouseEvent *event)
{

	if (m_isPress)
	{
		auto point = event->pos();
		auto movePoint = vec2(point.x(), point.y());
		auto trans = (movePoint - m_PressPoint);// *vec2(1.0 / m_nWidth, 1.0 / m_nHeight);
		trans.x /= m_nWidth;
		trans.y /=-m_nHeight;
		m_trans = trans + m_transDelta;
	}

}

void D3d11RenderWidget::wheelEvent(QWheelEvent *event)
{
	float fDelta = event->delta();
	if (fDelta < 0.0f) {
		m_scale = m_scale - 0.1f;
	}
	else {
		m_scale = m_scale + 0.1f;
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
			return false;
		}
		m_camera_or_video >> m_frame;
	}
	m_nWidth = m_frame.cols;
	m_nHeight = m_frame.rows;

	int width = m_frame.cols;
	int height = m_frame.rows;

	////��ʼ��ͼ����ģ��
	ccInitFilter(m_d3dDevice, m_d3dDevContext);

	m_pBGTexture = new DX11Texture();

	
	//������Ⱦ��Ŀ�������
	m_renderTargetTex = new DX11Texture();
	m_renderTargetTex->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, width, height);

	if (m_AR) {
		m_StickerEffect = ccEffectCreate();

		char* Rpath = (char*)"../BuiltInResource";
		ccEffectSetPath(m_StickerEffect, Rpath);

		ccEffectSetEffectZipSync(m_StickerEffect, "./resource/FaceBeatifulYYOld.zip", NULL, NULL, NULL);

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

	//���ر�������
	m_rectDraw.init(1, 1, "");


	//face point init
	ccFDInit("../3rdparty/FaSDK/faceModel/");
	//	ccFDSetFilterType(1);
	//	ccFD(0.8f);
	ccFDSetLandmarkFilter(0.5);
	//face expression control
	ccFDActivateEyeExpression();
	ccActivateStageTwo("../3rdparty/FaSDK/faceModel/");
	ccActivateStageTwoEye("../3rdparty/FaSDK/faceModel/");

	//hair segment
	if (m_showSegHair)
	{
		if (!ccHPActivateHairSeg("../3rdparty/ccHPSDK/models/hair_seg_v1.0.0")) {
			std::cout << "Activate HairSeg Faild!" << std::endl;
		}
		ccHPSetInputSize(width, height);
		ccHPGetHairMaskOutShape(&hair_out_w, &hair_out_h, &hair_out_c);
		if (m_HairMask == NULL)
		{
			m_HairMask = new byte[hair_out_w*hair_out_h*hair_out_c];
		}
	}
	//Body segment 
	if (m_showSegBody)
	{
		segment_net = CreateFurnaceInterface();
		
		//segment_net->setD3D11Environment(NULL, NULL);
		//segment_net->set_and_calc_segment_params(640, 480, save_height, pad_rate, min_pad, save_width, padded_w, padded_h);

		if (-1 == segment_net->load_segment_model("../3rdparty/BodyMask/mnnmodel_singo_x128.mnn","../3rdparty/BodyMask/mnnmodel_singo.mnn", width, height, &save_height, &save_width, 2))
		{
			cout << "load_segment_model error!" << endl;
		}
	}
	//hand point detect and Gesture Recognition
	if (m_showHand)
	{
		if (!ccHGInit("../3rdparty/cppsdk/models")){
			std::cout << "Activate Hand Model Faild!" << std::endl;
		}
		ccHGSetGestureLooseness(5.0); // 0.0~10.0之间取值
	}


	return S_OK;
}


HRESULT D3d11RenderWidget::VertexIndex()
{
	prepareRect();

	HRESULT hr = S_OK;

	m_d3dDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	{
		//������Ⱦ״̬
		D3D11_RASTERIZER_DESC rasterizer_desc;
		memset(&rasterizer_desc, 0, sizeof(D3D11_RASTERIZER_DESC));
		rasterizer_desc.CullMode = D3D11_CULL_NONE;
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		rasterizer_desc.DepthClipEnable = true;
		rasterizer_desc.FrontCounterClockwise = true;
		ID3D11RasterizerState *pRasterizerState = NULL;
		m_d3dDevice->CreateRasterizerState(&rasterizer_desc, &pRasterizerState);
		m_d3dDevContext->RSSetState(pRasterizerState);
		pRasterizerState->Release();

		//���ƻ��״̬
		D3D11_BLEND_DESC blend_desc;
		memset(&blend_desc, 0, sizeof(blend_desc));
		blend_desc.AlphaToCoverageEnable = false;
		blend_desc.IndependentBlendEnable = false;
		blend_desc.RenderTarget[0].BlendEnable = true;
		blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		ID3D11BlendState *pBlendState = NULL;
		m_d3dDevice->CreateBlendState(&blend_desc, &pBlendState);
		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		m_d3dDevContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);
		pBlendState->Release();


		//���״̬����
		D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
		memset(&depth_stencil_desc, 0, sizeof(depth_stencil_desc));
		depth_stencil_desc.DepthEnable = false;
		depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
		depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depth_stencil_desc.StencilEnable = FALSE;
		depth_stencil_desc.StencilReadMask = 0;
		depth_stencil_desc.StencilWriteMask = 0;

		ID3D11DepthStencilState *pDepthState = NULL;
		m_d3dDevice->CreateDepthStencilState(&depth_stencil_desc, &pDepthState);
		m_d3dDevContext->OMSetDepthStencilState(pDepthState, 1);
		pDepthState->Release();
	}
	return hr;
}

// int g_thresholds1 = 500;
// int g_thresholds2 = 440;
// int g_thresholds3 = 60;
// int g_thresholds4 = 800;
// //int g_thresholds5 = 500;
// 
// cv::Mat srcimage;
// void canny_track(int, void *)
// {
// 	if (srcimage.rows > 0 && srcimage.cols > 0)
// 		cv::imshow("����", srcimage);
// }

	float g_Alpha = 0.0;
void D3d11RenderWidget::Render()
{


	if (!m_isPause) {
		if (m_file_type == OpenFileType::CAMERA) {
			m_camera_or_video >> m_frame;
			cv::flip(m_frame, m_frame, 1);
		}
		if (m_file_type == OpenFileType::VIDEO_FILE) {
			m_camera_or_video >> m_frame;
			if (m_frame.empty()) { //ѭ��������Ƶ
				m_camera_or_video.set(CV_CAP_PROP_POS_FRAMES, 0);
				return;
			}
			//cv::flip(m_frame, m_frame, 1);
			//cv::transpose(m_frame, m_frame);
			
			Sleep(30);
			
		}
	}
	int width = m_frame.cols;
	int height = m_frame.rows;
	if (width*height == 0)return;

    cv::Mat s_frame = m_frame;

	m_pBuffer = (unsigned char *)s_frame.ptr();
	int nChannel = s_frame.channels();
	m_nWidth = s_frame.cols;
	m_nHeight = s_frame.rows;

	ccFDFaceRes_t faceRes;
	faceRes.numFace = 0;

	if (m_show_keypoints)
	{
		ccFDDetect(m_pBuffer, m_nWidth, m_nHeight, m_nWidth * nChannel, CT_BGR);
		ccFDGetFaceRes(&faceRes);
	}

	if (m_showHand)
	{
		double t1 = (double)cv::getTickCount();

		ccHGDetect(m_pBuffer, m_nWidth, m_nHeight, m_nWidth * nChannel, IM_BGR);

		//double t2 = (double)cv::getTickCount();
		//std::cout << "hand detect time:" << (t2 - t1) * 1000 / (cv::getTickFrequency()) << "  ms" << endl;
		ccHGHandRes resHand;
		ccHGGetHandRes(&resHand);

		//ccEffectSetHand(m_StickerEffect, &resHand);

	}
	//
	cvtColor(s_frame, s_frame, cv::COLOR_BGR2RGBA);
	m_pBuffer = (unsigned char *)s_frame.ptr();
	if (m_show_origin)
	{
//  		cv::Mat mLut = cv::imread("./ImageFilter/lookup_UE.png");
// 		cv::Mat mLutBase = cv::imread("./ImageFilter/lookup.png");
//    		cvtColor(mLut, mLut, cv::COLOR_BGR2RGBA);
//   		cvtColor(mLutBase, mLutBase, cv::COLOR_BGR2RGBA);

//  		static CImageBeautify CB;
// 		BasicTimer hd_timer;
// 		CB.FilterLUT(mLutBase.data, mLut.data, mLutBase.cols, mLutBase.rows);
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_RGBA2BGR);
// 		//cv::cvtColor(mLutBase, mLutBase, cv::COLOR_BGR2YCrCb);
// 		cv::imwrite("out.png", mLutBase);


// 		cv::Mat mLutBase4096 = cv::Mat(cv::Size(4096,64), CV_8UC4);
// 		for (int i=0;i<8;i++)
// 		{
// 			BYTE *pData = mLutBase.data;
// 			BYTE *pDataDst = mLutBase4096.data;
// 			for (int r = 0;r<64;r++)
// 			{
// 				for (int c=0;c<512;c++)
// 				{
// 					pDataDst[(r * 4096 + 512* i +c) * 4] = pData[((i * 64 + r) * 512 + c) * 4];
// 					pDataDst[(r * 4096 + +512 * i + c) * 4+1] = pData[((i * 64 + r) * 512 + c) * 4+1];
// 					pDataDst[(r * 4096 + +512 * i + c) * 4+2] = pData[((i * 64 + r) * 512 + c) * 4+2];
// 					pDataDst[(r * 4096 + +512 * i + c) * 4+3] = 255;
// 				}
// 			}
// 		}
// 		static CImageBeautify CB;
// 		BasicTimer hd_timer;
// 		CB.FilterLUT(mLutBase4096.data, mLut.data, 4096, 64);
// 		cvtColor(mLutBase4096, mLutBase4096, cv::COLOR_RGBA2BGR);
// 		cv::imwrite("out.png", mLutBase4096);

// 		cv::Mat mLut = cv::imread("./ImageFilter/lookup2.png");
// 		cvtColor(mLut, mLut, cv::COLOR_BGR2RGBA);
// 		cv::Mat mLutBase = cv::imread("./ImageFilter/lookup.png");
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_YCrCb2RGB);
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_RGB2RGBA);
// 		static CImageBeautify CB;
// 		BasicTimer hd_timer;
// 		CB.FilterLUT(mLutBase.data, mLut.data, mLutBase.cols, mLutBase.rows);
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_RGBA2RGB);
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_RGB2YCrCb);
// 		cv::imwrite("out.png", mLutBase);


// 		cv::Mat mLut = cv::imread("./ImageFilter/out.png");
// 		cvtColor(mLut, mLut, cv::COLOR_RGB2BGRA);
// 		cv::Mat mLutBase = cv::imread("./1.png");
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_BGR2YCrCb);
//  		cvtColor(mLutBase, mLutBase, cv::COLOR_BGR2RGBA);
// 		static CImageBeautify CB;
// 		BasicTimer hd_timer;
// 		CB.FilterLUT(mLutBase.data, mLut.data, mLutBase.cols, mLutBase.rows);
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_RGBA2BGR);
// 		cvtColor(mLutBase, mLutBase, cv::COLOR_YCrCb2BGR);
// 		cv::imwrite("out.png", mLutBase);
	}


	//��������ͼ�����ڼ��
	{
		unsigned char *pImageSource = m_pBuffer;

			//д��landmark�㵽��������������ʾ
			if (1) {
				for (int i = 0; i < faceRes.numFace; ++i) {
					for (int j = 0; j < 118; ++j) {
// 						if (j==60||j==59||j==69||j==70)
// 						{
// 							continue;
// 						}
						ccFDPoint pt = faceRes.arrFace[i].arrShapeDense[j];
						int x = int(pt.x);
						int y = int(pt.y);
						unsigned char *pTemp = pImageSource + m_nWidth * 4 * y + x * 4;
						for (int dx = -1; dx <= 1; ++dx) {
							for (int dy = -1; dy <= 1; ++dy) {
								unsigned char *pWrite = pTemp + dy*m_nWidth * 4 + dx * 4;
								if (y + dy > 2 && y + dy < m_nHeight - 1 && x + dx > 2 && x + dx < m_nWidth - 2) {
									pWrite[0] = 0;
									pWrite[1] = 255;
									pWrite[2] = 0;
									pWrite[3] = 255;
								}
							}
						}
					}

				}
			}
			m_pBGTexture->initTexture(DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_BIND_SHADER_RESOURCE, m_nWidth, m_nHeight, pImageSource, m_nWidth * 4);
			m_rectDraw.setShaderTextureView(m_pBGTexture->getTexShaderView());

	}
	cvtColor(s_frame, s_frame, cv::COLOR_BGRA2RGBA);
	m_pBuffer = (unsigned char *)s_frame.ptr();
	ccEffectSetBGRA(m_StickerEffect, m_pBuffer);

// 	cv::namedWindow("����", cv::WINDOW_NORMAL);
// 	cv::createTrackbar("����", "����", &g_thresholds1, 1000, canny_track);//����������
// 	cv::createTrackbar("ĥƤ", "����", &g_thresholds4, 1000, canny_track);//����������
// 	cv::createTrackbar("similarity��", "����", &g_thresholds2, 1000, canny_track);//����������
// 	cv::createTrackbar("smoothness��", "����", &g_thresholds3, 1000, canny_track);//����������
	if (m_show_origin)
	{
		if (m_AR) {
			//enable seg body mask
			if (m_showSegBody)
			{
				unsigned char* mask = segment_net->detect_segment(m_frame.cols, m_frame.rows, m_frame.ptr(), BGR888, frame_index);
				frame_index++;
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

 			//cv::Mat ImgMask = cv::Mat(save_height, save_width, CV_8UC1, mask);
			//BasicTimer hd_timer;
  	//		if (ccPlayGiftEffectExt2(m_pBGTexture->getTexShaderView(), m_renderTargetTex->getTex(), m_pBGTexture->width(), m_pBGTexture->height(), &faceRes,NULL))
  	//		{
  	//			m_rectDraw.setShaderTextureView(m_renderTargetTex->getTexShaderView());
  	//		}
			g_Alpha = 1.0;
			//g_Alpha += 0.01;
			if (g_Alpha>1)
			{
				g_Alpha = 0;
			}

			ccEffectSetAlpha(m_StickerEffect, 1.0, SMOOTH_EFFECT);
		//	ccEffectSetAlpha(m_StickerEffect, 0.5, FACE_YAMANE_EFFECT);
		//	ccEffectSetAlpha(m_StickerEffect, 0.0, RHYTHM_EFFECT);
			if (ccEffectProcessTexture(m_StickerEffect,m_pBGTexture->getTexShaderView(), m_renderTargetTex->getTex(), m_pBGTexture->width(), m_pBGTexture->height(), &faceRes))
			{
				//ccRenderMatting(m_StickerEffect, m_pBGTexture->getTexShaderView(), m_renderTargetTex->getTex(), m_pBGTexture->width(), m_pBGTexture->height(), 0.3);
				if (m_show_Segment)
				{
					cv::Mat img = cv::imread("./ImageFilter/rouwu.png",-1);
					cvtColor(img, img, cv::COLOR_BGRA2RGBA);
					ccEffectSetMask(m_StickerEffect, img.data, img.cols, img.rows, FACE_EYEBROWSTRICK);

					img = cv::imread("./ImageFilter/maoyan.png",-1);
					cvtColor(img, img, cv::COLOR_BGRA2RGBA);
					ccEffectSetMask(m_StickerEffect, img.data, img.cols, img.rows, FACE_EYESTRICK);

					img = cv::imread("./ImageFilter/naizui.png",-1);
					cvtColor(img, img, cv::COLOR_BGRA2RGBA);
					ccEffectSetMask(m_StickerEffect, img.data, img.cols, img.rows, FACE_BLUSH);

					ccEffectSetColor(m_StickerEffect, 83, 10, 10, 255);
					//ccRenderMattingAfterEffect(m_StickerEffect, m_renderTargetTex->getTex());
				}
				else
				{
					ccEffectSetAlpha(m_StickerEffect, 0, TYPE_FILTER_EFFECT);
				}
				m_rectDraw.setShaderTextureView(m_renderTargetTex->getTexShaderView());
				//m_pBGTexture->ReadTextureToCpu(m_pBuffer);
			}
			else
			{
				m_rectDraw.setShaderTextureView(m_pBGTexture->getTexShaderView());
				
			}
			//DeviceContextPtr->FinishCommandList(1, 0);
// 			float tt = hd_timer.UpdateAndGetTotal();
// 			printf("Next cost %.2fms\n", tt);
		} 
		
		else {
			//ccFacialAnimationUpdateAvantarParam(m_StickerEffect, res.arrAlpha);
			//res.numFace = 0;
			if (ccFacialAnimationProcessTextureExt(m_StickerEffect, m_pBGTexture->getTexShaderView(), m_renderTargetTex->getTex(),\
				m_pBGTexture->width(), m_pBGTexture->height(), &faceRes)) {
				m_rectDraw.setShaderTextureView(m_renderTargetTex->getTexShaderView());
			} else {
				m_rectDraw.setShaderTextureView(m_pBGTexture->getTexShaderView());
			}
			
		}
	}

	D3D11_VIEWPORT vp;
	vp.Width = this->width();
	vp.Height = this->height();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // rgba  
	m_d3dDevContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	DeviceContextPtr->ClearRenderTargetView(m_renderTargetView, ClearColor);
	DeviceContextPtr->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_d3dDevContext->RSSetViewports(1, &vp);
	float x_s = 1;
	float y_s = 1;
	getScale(x_s, y_s);

// 	m_rectDraw.init(1, 1, "BG.png");
// 	m_rectDraw.render(m_trans, vec2(m_scale * x_s, m_scale *y_s), 0, this->width(), this->height());
// 
// 	m_rectDraw.setShaderTextureView(m_renderTargetTex->getTexShaderView());

	{
//		BasicTimer hd_timer;
//		m_renderTargetTex->ReadTextureToCpu(m_pBuffer);
//		float tt = hd_timer.UpdateAndGetTotal();
// 		cvtColor(s_frame, s_frame, cv::COLOR_BGRA2RGBA);
// 		cv::imwrite("result.png", s_frame);
// 		printf("ReadTextureToCpu %.2fms\n", tt);
	}


	auto blender = DXUtils::CreateBlendState();
	float blendFactor[] = { 0.f,0.f,0.f,0.f };
	DeviceContextPtr->OMSetBlendState(blender, blendFactor, 0xffffffff);
	m_rectDraw.render(m_trans, vec2(m_scale * x_s, m_scale *y_s), 0, this->width(), this->height());

	m_swapChain->Present(0, 0);
}

//--------------------------------------------------------------------------------------  
// Create Direct3D device and swap chain  
//--------------------------------------------------------------------------------------  
HRESULT D3d11RenderWidget::InitDevice()
{
	HRESULT hr = S_OK;

	//RECT rc;
	//GetClientRect(g_hWnd, &rc);
	//width = rc.right - rc.left;
	//height = rc.bottom - rc.top;
	//cout << width << "," << height << endl;
	UINT createDeviceFlags = 0;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width();
	sd.BufferDesc.Height = height();
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = (HWND)winId();
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		D3D_DRIVER_TYPE driverType = driverTypes[driverTypeIndex];
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		hr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_swapChain, &m_d3dDevice, &featureLevel, &m_d3dDevContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view  
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	//�������ģ�建����
	D3D11_TEXTURE2D_DESC depthStencil_desc;
	memset(&depthStencil_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));
	memset(&depthStencil_desc, 0, sizeof(depthStencil_desc));
	depthStencil_desc.ArraySize = 1;
	depthStencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencil_desc.CPUAccessFlags = 0;
	depthStencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencil_desc.Width = width();
	depthStencil_desc.Height = height();
	depthStencil_desc.MipLevels = 1;
	depthStencil_desc.MiscFlags = 0;
	depthStencil_desc.Usage = D3D11_USAGE_DEFAULT;
	depthStencil_desc.SampleDesc.Count = 1;
	depthStencil_desc.SampleDesc.Quality = 0;
	ID3D11Texture2D *pTextureDepthStencil = NULL;
	m_d3dDevice->CreateTexture2D(&depthStencil_desc, NULL, &pTextureDepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	memset(&dsv_desc, 0, sizeof(dsv_desc));
	dsv_desc.Format = depthStencil_desc.Format;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Texture2D.MipSlice = 0;
	m_d3dDevice->CreateDepthStencilView(pTextureDepthStencil, &dsv_desc, &m_depthStencilView);
	pTextureDepthStencil->Release();

	m_d3dDevContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
	//g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

	// Setup the viewport  
	D3D11_VIEWPORT vp;
	vp.Width = width();
	vp.Height = height();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_d3dDevContext->RSSetViewports(1, &vp);

	return S_OK;
}