#include "glrenderwiget.h"
#include <QKeyEvent>
#include "Toolbox/GL/openglutil.h"
#include "Toolbox/GL/DoubleBuffer.h"
#include <windows.h>
#include "CC3DEngine/Common/CC3DEnvironmentConfig.h"
#include <playsoundapi.h>
#include <mmeapi.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Toolbox/inc.h"

#pragma comment(lib,"winmm.lib")
using namespace std;
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

class VideoCaptureOpenCV : public VideoCaptureMapper
{
public:
	VideoCaptureOpenCV()
	{
		m_decoder.reset(new cv::VideoCapture());
	}
	virtual ~VideoCaptureOpenCV()
	{
		Close();
	}

	virtual int Open(const char* file) override
	{
		return m_decoder->open(0);
		// 		m_decoder->open(2);
		// 		m_decoder->set(cv::CAP_PROP_FRAME_HEIGHT, 1080.0);
		// 		m_decoder->set(cv::CAP_PROP_FRAME_WIDTH, 1920.0);
		// 		return 1;
	}

	virtual int Start() override
	{
		return m_decoder->isOpened();
	}

	virtual int Width() override
	{
		return static_cast<int>(m_decoder->get(3));
	}

	virtual int Height() override
	{
		return static_cast<int>(m_decoder->get(4));
	}

	virtual unsigned char* GetNextFrame() override
	{
		cv::Mat frame;
		m_decoder->read(frame);
		cv::flip(frame, frame, 1);
		nextFrame.release();
		nextFrame = cv::Mat(frame.size(), CV_8UC4, cv::Scalar(0));
		cv::cvtColor(frame, nextFrame, CV_BGR2BGRA, 4);
		//SaveJpeg3C("tmp1.jpg", frame.data, frame.cols, frame.rows);
		//SaveJpeg4C("tmp2.jpg", nextFrame.data, nextFrame.cols, nextFrame.rows);
		return nextFrame.data;
	}

	virtual double GetTimeStamp()
	{
		return 0.0;
	}

	virtual int Close() override
	{
		m_decoder->release();
		return !m_decoder->isOpened();
	}

	virtual double GetFps() override { return 0; }
	virtual int OpenFromMemory(char* buffer, size_t buffer_size) { return 0; }
	virtual double GetDuration() { return 0.f; }
	virtual unsigned char* GetFrameByTimeStamp(double timeStamp) override { return NULL; }

protected:
	std::shared_ptr<cv::VideoCapture> m_decoder;
	cv::Mat nextFrame;
};



glRenderWiget::glRenderWiget(QWidget *parent) : QWidget(parent)
{
    m_show_keypoints = false;
    m_show_origin = false;
    m_pBuffer = NULL;
    m_pBGTexture = 0;
    m_renderTargetTex =0;
    m_InterFace = NULL;
	m_VideoCapture = nullptr;

	m_FaceDet = NULL;
	setFocusPolicy(Qt::StrongFocus);
}

glRenderWiget::~glRenderWiget()
{
    GL_DELETE_TEXTURE(m_pBGTexture);
    GL_DELETE_TEXTURE(m_renderTargetTex);
    SAFE_DELETE_ARRAY(m_pBuffer);
    SAFE_DELETE(m_InterFace);
	SAFE_DELETE(m_VideoCapture);

	SAFE_DELETE(m_FaceDet);
	SAFE_DELETE(m_ExpressDrive);
	SAFE_DELETE_ARRAY(frame_wav_buffer);
	cap.release();
}


bool glRenderWiget::changeImageFileType(QString path)
{
	bVideo = false;
	std::string str_path = path.toStdString();
	cv_img = cv::imread(str_path);
	if (cv_img.empty()) 
	{
		return false;
	}
	GL_DELETE_TEXTURE(m_renderTargetTex);
	if (m_FaceDet == NULL)
	{
		m_FaceDet = new FaceDetector();
	}
	SAFE_DELETE(m_ExpressDrive);
	if (m_ExpressDrive == NULL)
	{
		m_ExpressDrive = new CC3DExpressDrive();
		m_ExpressDrive->SetModelPath("../3rdparty/FacialAnimation/3DModels/model_74.endat");
	}
	return true;
}

bool glRenderWiget::changeCamera()
{
	bVideo = true;
	SAFE_DELETE(m_VideoCapture);
	m_VideoCapture = new VideoCaptureOpenCV();
	m_VideoCapture->Close();
	if (!m_VideoCapture->Open(NULL))
	{
		LOGE("Counld no open camera");
		return FALSE;
	}
	m_VideoCapture->Start();
	if (m_FaceDet == NULL)
	{
		m_FaceDet = new FaceDetector();
	}
	SAFE_DELETE(m_ExpressDrive);
	if (m_ExpressDrive == NULL)
	{
		m_ExpressDrive = new CC3DExpressDrive();
		m_ExpressDrive->SetModelPath("../3rdparty/FacialAnimation/3DModels/model_74.endat");
	}

	GL_DELETE_TEXTURE(m_renderTargetTex);
	return true;
}

bool glRenderWiget::changeVideoFileType(QString path)
{
	bVideo = true;
	std::string video_path = path.toStdString();

	SAFE_DELETE(m_VideoCapture);
	m_VideoCapture = new VideoCaptureFFmpeg();
	std::string chFile = path.toLocal8Bit().constData();
	int ret = m_VideoCapture->Open(chFile.c_str());
	if (ret < 0)
	{
		LOGE("Counld no open %s", chFile);
		return false;
	}

	if (m_FaceDet == NULL)
	{
		m_FaceDet = new FaceDetector();
	}
	SAFE_DELETE(m_ExpressDrive);
	if (m_ExpressDrive == NULL)
	{
		m_ExpressDrive = new CC3DExpressDrive();
		m_ExpressDrive->SetModelPath("../3rdparty/FacialAnimation/3DModels/model_74.endat");
	}
	GL_DELETE_TEXTURE(m_renderTargetTex);

	return true;
}

//bool glRenderWiget::changeAudioFileType(QString path)
//{
//	if (infile != nullptr)
//	{
//		sf_close(infile);
//	}
//
//	std::string chFile = path.toLocal8Bit().constData();
//	memset(&sfinfo, 0, sizeof(sfinfo));
//	if (!(infile = sf_open(chFile.c_str(), SFM_READ, &sfinfo))) {	/* Open failed so print an error message. */
//		printf("Not able to open input file %s.\n", chFile);
//		/* Print the error message from libsndfile. */
//		puts(sf_strerror(NULL));
//		return 1;
//	};
//
//	kWindowLength = sfinfo.samplerate / 100;
//
//	frame_wav_buffer = new float[48000];
//	memset(frame_wav_buffer, 0, sizeof(float) * 48000);
//	m_InterFace->SetAudio(sfinfo.samplerate);
//
//	PlayPCMSound(chFile);
//	wavFileLoaded = true;
//}

void testCallback(int event, const void *lpParam)
{
	if (event == 1)
	{
		std::cout << "test callback success!!!!" << event << endl;
	}
	
}

 bool glRenderWiget::changeEffectResource(QString path)
 {
     if(m_InterFace !=NULL)
     {
         m_InterFace->Realese();
     }
     else
     {
         m_InterFace = new CCEffectInterfaceGL();
		 char Rpath[] = "../BuiltInResource";
		 m_InterFace->SetResourcePath(Rpath);
		 
		 const char* szXML = 0;
		 m_InterFace->loadBaseFromZipSync("../Resource/FaceBeautiful3.zip", "test.xml");
     }
	 const char* pFileName = path.toLocal8Bit();
	 m_InterFace->loadEffectFromZipAsyn(pFileName, "test.xml", nullptr, nullptr);
	 return  true;

 }

 HANDLE hThread = nullptr;
 HANDLE hEvent = nullptr;

 wchar_t* C2W(const char* str)
 {
	 int len = MultiByteToWideChar(CP_OEMCP, 0, str, -1/*null terminated*/, NULL, 0);
	 wchar_t* wstr = new wchar_t[len + 1];
	 MultiByteToWideChar(CP_OEMCP, 0, str, -1/*null terminated*/, wstr, len);
	 return wstr;
 }

void glRenderWiget::paintEvent(QPaintEvent *event)
{

}

void glRenderWiget::resizeEvent(QResizeEvent *event)
{

}

void glRenderWiget::onTogglePausePlay() {
	flag = !flag;
}

void glRenderWiget::ZoomInOut(float scale)
{
	//m_InterFace->ZoomInOut(scale);
}

void glRenderWiget::Render()
{
	//if (wavFileLoaded)
	//{
	//	CC3DEnvironmentConfig::getInstance()->musicPlayed = true;
	//	float frame_ms = CC3DEnvironmentConfig::getInstance()->deltaTime * 1000;
	//	int sample_count = (int)(frame_ms * (kWindowLength / 10));

	//	int num_samples = 0;
	//	num_samples = sf_read_float(infile, frame_wav_buffer, sample_count); //目前只支持单通道音频分析

	//	if (num_samples == sample_count)
	//	{
	//		int nSample = floor(frame_ms);
	//		m_InterFace->AnalyseAudio(frame_wav_buffer, num_samples);
	//	}
	//	else
	//	{
	//		wavFileLoaded = false;
	//		sf_close(infile);
	//		m_InterFace->EndAudio();
	//	}

	//	memset(frame_wav_buffer, 0, sizeof(float) * 48000);
	//}

	if (KEY_DOWN(VK_RBUTTON))
	{
		flag = !flag;
	}
    if(m_InterFace!=NULL)
    {
		if (flag)
		{
			if (m_VideoCapture != nullptr && bVideo)
			{
				byte_t* pData = nullptr;
				pData = m_VideoCapture->GetNextFrame();
				int m_Width = m_VideoCapture->Width();
				int m_Height = m_VideoCapture->Height();

				if (pData)
				{
					frame = cv::Mat(m_Height, m_Width, CV_8UC4, pData);
					cv::cvtColor(frame, frame, CV_BGRA2BGR);
				}
				else
				{
					m_VideoCapture->Start();
					return;
				}
			}
			else
			{
				frame = cv_img;
			}
			//cv::resize(frame, frame, cv::Size( frame.cols * 0.5, frame.rows * 0.5));
			m_FaceDet->getFaceRes(frame);
			//m_FaceDet->ShowImg(frame);
			//cout << "detect face:" << m_FaceDet->m_faceRes->numFace << endl;
			m_ExpressDrive->GetBlendshapeWeights(frame.data, frame.cols, frame.rows,(Vector2*)m_FaceDet->m_faceRes->arrFace[0].arrShapeDense);
			m_InterFace->SetExpression(m_ExpressDrive->GetCoeffs());

			m_nWidth = frame.cols;
			m_nHeight = frame.rows;
			cv::Mat frame_rgba;
			cv::cvtColor(frame, frame_rgba, CV_BGR2RGBA);
			
			if (m_show_keypoints)
			{
				m_FaceDet->DrawPoints(frame_rgba);
			}
	
			GL_DELETE_TEXTURE(m_pBGTexture);
			OpenGLUtil::createToTexture(m_pBGTexture, m_nWidth, m_nHeight, frame_rgba.ptr());


			if (m_renderTargetTex == 0)
			{
				OpenGLUtil::createToTexture(m_renderTargetTex, m_nWidth, m_nHeight, frame_rgba.ptr());
			}
			if (m_show_origin)
			{
				m_showTexture = m_pBGTexture;
			}
			else
			{
				m_showTexture = m_renderTargetTex;
			}

			if (!m_show_origin &&!m_InterFace->renderEffectToTexture(m_pBGTexture, m_renderTargetTex, m_nWidth, m_nHeight, m_FaceDet->m_faceRes))
			{
				m_showTexture = m_pBGTexture;
				
			}


			auto end = std::chrono::system_clock::now();

			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - lastTimeStamp);
			//cout << double(duration.count()) * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << endl;

			lastTimeStamp = end;

			//printf("%f \n", CC3DEnvironmentConfig::getInstance()->deltaTime);

			//保存输出Texture为图像
			auto faceInfo = m_FaceDet->m_faceRes;

			if (faceInfo->numFace > 0)
			{
				//CCFrameBuffer *m_pFBOA = m_InterFace->m_DoubleBuffer->m_pFBOA;
				//BYTE *pResult = (BYTE*)m_pFBOA->ReadPixels();
				//cv::Mat mImg = cv::Mat(m_nHeight, m_nWidth, CV_8UC4, pResult);
				//cv::cvtColor(mImg, mImg, CV_RGBA2BGRA);
				//std::string result_path = "result.png";
				//cv::imwrite(result_path, mImg);
			}

		}

	}
}
