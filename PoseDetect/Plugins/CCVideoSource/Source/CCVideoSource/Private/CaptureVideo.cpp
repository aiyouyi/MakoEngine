#include "CaptureVideo.h"
#include<vector>
#include<string>
#include<thread>
#include<iostream>
#include <fstream>
#include "LibYUVWrapper.h"
#include "core/vec2.h"
#include "cameracommfunc.h"
#include "core/logger.h"
#include <Runtime/Core/Public/Misc/Paths.h>
#include <Runtime/Core/Public/HAL/PlatformFilemanager.h>
#include <Runtime/core/Public/GenericPlatform/GenericPlatformFile.h>

#ifndef FALSE
#define  FALSE 0
#endif

#ifndef TRUE
#define  TRUE 1
#endif


const GUID MEDIASUBTYPE_I420 =
{ 0x30323449, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

SampleGrabberCallback g_sampleGrabberCB;  //CallBack

std::wstring CaptureVideo::SaveDir = L"";
std::wstring CaptureVideo::UID = L"";

class StaticClassFunction
{
public:

	static void FreeMediaType(AM_MEDIA_TYPE& mt)
	{
		if (mt.cbFormat != 0) {
			CoTaskMemFree((LPVOID)mt.pbFormat);
			mt.cbFormat = 0;
			mt.pbFormat = nullptr;
		}

		if (mt.pUnk) {
			mt.pUnk->Release();
			mt.pUnk = nullptr;
		}
	}

	static  void DeleteMediaType(AM_MEDIA_TYPE* pmt)
	{
		if (pmt != NULL)
		{
			FreeMediaType(*pmt);
			CoTaskMemFree(pmt);
		}
	}
};

HRESULT setCaptureResolution(ICaptureGraphBuilder2* pGraphicBuilder, IBaseFilter* deviceFilter, core::vec2i resolution, UINT64 frameInterval, GUID expectedMediaType)
{
	IAMStreamConfig* pSC;

	HRESULT hResult = pGraphicBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video, deviceFilter,
		IID_IAMStreamConfig, (void**)&pSC);

	if (S_OK != hResult || !pSC)
	{
		return hResult;
	}

	int iCount = 0, iSize = 0;
	hResult = pSC->GetNumberOfCapabilities(&iCount, &iSize);
	if (S_OK != hResult)
	{
		return hResult;
	}

	hResult = E_FAIL;
	// Check the size to make sure we pass in the correct structure.
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		// Use the video capabilities structure.
		std::vector<AM_MEDIA_TYPE*> vecMediaTypes;
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			HRESULT hr = pSC->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				/* Examine the format, and possibly use it. */

				// Delete the media type when you are done.

				if ((pmtConfig->majortype == MEDIATYPE_Video) &&
					(pmtConfig->subtype == expectedMediaType) &&
					(pmtConfig->formattype == FORMAT_VideoInfo) &&
					(pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
					(pmtConfig->pbFormat != NULL))
				{
					VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					// pVih contains the detailed format information.
					LONG lWidth = pVih->bmiHeader.biWidth;
					LONG lHeight = pVih->bmiHeader.biHeight;

					if (lWidth == resolution.cx && lHeight == resolution.cy)
					{
						hResult = S_OK;
						pVih->AvgTimePerFrame = frameInterval;
						hResult = pSC->SetFormat(pmtConfig);
						StaticClassFunction::DeleteMediaType(pmtConfig);
						break;
					}

				}

				StaticClassFunction::DeleteMediaType(pmtConfig);
			}
		}

		return hResult;
	}

	return E_FAIL;
}

HRESULT CaptureVideo::selectVideoDeviceByName(const std::wstring& deviceName, IBaseFilter** pOutFilter)
{
	HRESULT hr = S_OK;
	*pOutFilter = NULL;
	ICreateDevEnum* pCreateDevEnum;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (LPVOID*)&pCreateDevEnum);
	if (!pCreateDevEnum || FAILED(hr))
	{
		return hr;
	}

	IEnumMoniker * pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

	if (!pEm || FAILED(hr))
	{
		return hr;
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	 IMoniker * pM;
	hr = S_OK;

	while ((hr = pEm->Next(1, &pM, &ulFetched)) == S_OK)
	{

		IPropertyBag * pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (hr != S_OK || !pBag)
		{
			continue;
		}

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read(L"FriendlyName", &devVar, NULL);
		if (hr != S_OK)
		{
			pM->Release();
			continue;
		}
		std::wstring friendlyName = devVar.bstrVal;
		::VariantClear(&devVar);

		std::wstring devPathName;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read(L"DevicePath", &devVar, NULL);
		if (hr == S_OK )
		{
			devPathName = devVar.bstrVal;
			::VariantClear(&devVar);
		}
		else
		{
			devPathName = friendlyName;
		}
		if (deviceName==L"" || friendlyName.find(deviceName)!=std::wstring::npos)
		{
			m_CameraName = friendlyName;
			//IBaseFilter* pCap;
			hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pOutFilter);
			return S_OK;
		}
		pM->Release();
	}
	return E_FAIL;
}


CaptureVideo::CaptureVideo()
{
	//COM Library Initialize
	if (FAILED(CoInitialize(NULL)))
	{
		return;
	}

	//initialize member variable
	m_pDevFilter = NULL;
	m_pCaptureGB = NULL;
	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaEvent = NULL;
	m_pSampGrabber = NULL;
	m_bConnect = FALSE;
	InitializeEnv();
	g_sampleGrabberCB.ReceiveDataCallBack.bind(std::bind(&CaptureVideo::SGReceiveDataCallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),this );
}

CaptureVideo::~CaptureVideo()
{
	g_sampleGrabberCB.ReceiveDataCallBack.unbind(this);
	CloseInterface();
	CoUninitialize();
}

HRESULT CaptureVideo::EnumAllDevices(HWND hCombox)
{
	return S_OK;
}

HRESULT CaptureVideo::InitializeEnv()
{
	HRESULT hr;

	//Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,
						  IID_IGraphBuilder,(LPVOID*)&m_pGraphBuilder);
	if(FAILED(hr))
		return hr;
	
	//Create the capture graph builder
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,
						  IID_ICaptureGraphBuilder2,(LPVOID*)&m_pCaptureGB);
	if(FAILED(hr))
		return hr;

	//Obtain interfaces for media control and Video Window
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl,(LPVOID*)&m_pMediaControl);
	if(FAILED(hr))
		return hr;

	hr = m_pGraphBuilder->QueryInterface(IID_IMediaEventEx,(LPVOID*)&m_pMediaEvent);
	if(FAILED(hr))
		return hr;


	if(FAILED(hr))
		return hr;
	m_pCaptureGB->SetFiltergraph(m_pGraphBuilder);
	if(FAILED(hr))
		return hr;
	return hr;
}

void CaptureVideo::CloseInterface()
{

	if (m_pMediaControl)
		m_pMediaControl->Stop();


	if(m_pMediaEvent)
		m_pMediaEvent->SetNotifyWindow(NULL,WM_GRAPHNOTIFY,0);

	//release interface
	ReleaseInterface(m_pDevFilter);
	ReleaseInterface(m_pCaptureGB);
	ReleaseInterface(m_pGraphBuilder);
	ReleaseInterface(m_pMediaControl);
	ReleaseInterface(m_pMediaEvent);
	ReleaseInterface(m_pSampGrabber);
}

HRESULT CaptureVideo::BindFilter(int deviceID, IBaseFilter **pBaseFilter)
{
	ICreateDevEnum *pDevEnum;
	IEnumMoniker   *pEnumMon;
	IMoniker	   *pMoniker;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,(LPVOID*)&pDevEnum);
	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnumMon, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;
			return hr;
		}
		pEnumMon->Reset();
		ULONG cFetched;
		int index = 0;
		while(hr=pEnumMon->Next(1,&pMoniker,&cFetched),hr == S_OK, index<= deviceID)
		{
			IPropertyBag *pProBag;
			hr = pMoniker->BindToStorage(0,0,IID_IPropertyBag,(LPVOID*)&pProBag);
			if (SUCCEEDED(hr))
			{
				if (index == deviceID)
				{
					pMoniker->BindToObject(0,0,IID_IBaseFilter,(LPVOID*)pBaseFilter);
				}
			}
			pMoniker->Release();
			index++;
		}
		pEnumMon->Release();
	}
	return hr;
}

HRESULT CaptureVideo::SetupVideoWindow(LONG nLeft, LONG nTop, LONG nWidth, LONG nHeight)
{
	return 0;
}

void CaptureVideo::ResizeVideoWindow(LONG nLeft, LONG nTop, LONG nWidth, LONG nHeight)
{
}

HRESULT CaptureVideo::OpenDevice(const std::wstring& deviceName)
{
	HRESULT hr;
	IBaseFilter *pSampleGrabberFilter;
	if (m_bConnect)
	{
		CloseInterface();
		InitializeEnv();
	}

	hr = CoCreateInstance(CLSID_SampleGrabber,NULL,CLSCTX_INPROC_SERVER,
						  IID_IBaseFilter, (LPVOID*)&pSampleGrabberFilter);
	if(FAILED(hr))
	{
		return hr;
	}
	//bind device filter
	hr =  selectVideoDeviceByName(deviceName, &m_pDevFilter);
	//hr = BindFilter(deviceID,&m_pDevFilter);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGraphBuilder->AddFilter(m_pDevFilter,L"Video Filter");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraphBuilder->AddFilter(pSampleGrabberFilter,L"Sample Grabber");
	if (FAILED(hr))
	{
		return hr;
	}

	hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber,(LPVOID*)&m_pSampGrabber);
	if(FAILED(hr))
	{
		return hr;
	}

	//set media type
	AM_MEDIA_TYPE mediaType;
	ZeroMemory(&mediaType,sizeof(AM_MEDIA_TYPE));
	//Find the current bit depth
	HDC hdc=GetDC(NULL);
	int iBitDepth=GetDeviceCaps(hdc, BITSPIXEL);
	g_sampleGrabberCB.m_iBitCount = iBitDepth;
	ReleaseDC(NULL,hdc);
	//Set the media type
	mediaType.majortype = MEDIATYPE_Video;
	switch(iBitDepth)
	{
	case  8:
		mediaType.subtype=MEDIASUBTYPE_RGB8;
		break;
	case 16:
		mediaType.subtype=MEDIASUBTYPE_RGB555;
		break;
	case 24:
		mediaType.subtype=MEDIASUBTYPE_RGB24;
		break;
	case 32:
		mediaType.subtype=MEDIASUBTYPE_RGB32;
		break;
	default:
		return E_FAIL;
	}
	mediaType.formattype = FORMAT_VideoInfo;

	core::vec2i targetRes = {1280,720};
	std::vector<core::vec2i> videoRes;
	VACameraCommFunc::getCaptureResolution(m_pCaptureGB, m_pDevFilter, videoRes);
	if (videoRes.empty())
	{
		return E_FAIL;
	}
	std::sort(videoRes.begin(), videoRes.end(),[](core::vec2i left, core::vec2i right){return left.x >= right.x && left.y >= right.y;});

	auto it = std::find(videoRes.begin(), videoRes.end(), targetRes);
	if (it == videoRes.end())
	{
		targetRes = videoRes[0];
	}

	
	UINT64 frameInterval = UINT64(10000000.0 / double(60));
	expectedMediaType = GetCapturePreferredMediaType2();

	FString temp =TEXT("");

	if (expectedMediaType == MEDIASUBTYPE_MJPG )
	{
		temp = TEXT("MEDIASUBTYPE_MJPG");
	}
	else  if (expectedMediaType == MEDIASUBTYPE_YUY2)
	{
		temp = TEXT("MEDIASUBTYPE_YUY2");
	}
	else if (expectedMediaType == MEDIASUBTYPE_YV12)
	{
		temp = TEXT("MEDIASUBTYPE_YV12");
	}
	else if(expectedMediaType == MEDIASUBTYPE_I420)
	{
		temp = TEXT("MEDIASUBTYPE_I420");
	}
	else if(expectedMediaType == MEDIASUBTYPE_RGB32)
	{
		temp = TEXT("MEDIASUBTYPE_RGB32");
	}
	else if(expectedMediaType == MEDIASUBTYPE_RGB24)
	{
		temp = TEXT("MEDIASUBTYPE_RGB24");
	}
	else if (expectedMediaType == MEDIASUBTYPE_ARGB32)
	{
		temp = TEXT("MEDIASUBTYPE_ARGB32");
	}

	setCaptureResolution(m_pCaptureGB, m_pDevFilter, targetRes, frameInterval, expectedMediaType);

	hr = m_pCaptureGB->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pDevFilter, NULL, pSampleGrabberFilter);
	if(FAILED(hr))
	{
		return hr;
	}

	hr = m_pSampGrabber->GetConnectedMediaType(&mediaType);
	if(FAILED(hr))
	{
		return hr;
	}


	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mediaType.pbFormat;
	g_sampleGrabberCB.m_lWidth = vih->bmiHeader.biWidth;
	g_sampleGrabberCB.m_lHeight = vih->bmiHeader.biHeight;
	// Configure the Sample Grabber
	hr = m_pSampGrabber->SetOneShot(FALSE);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pSampGrabber->SetBufferSamples(TRUE);
	if (FAILED(hr))
	{
		return hr;
	}
	// 1 = Use the BufferCB callback method.
	hr = m_pSampGrabber->SetCallback(&g_sampleGrabberCB,1);

	hr = m_pMediaControl->Run();
	if(FAILED(hr))
	{
		return hr;
	}

	CCamProperty CamProperty;
	if (ReadPropertyFromFile(m_CameraName, CamProperty))
	{
		SetProperty(CamProperty);
	}
	if (mediaType.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mediaType.pbFormat);
		mediaType.cbFormat = 0;
		mediaType.pbFormat = NULL;
	}
	if (mediaType.pUnk != NULL)
	{
		mediaType.pUnk->Release();
		mediaType.pUnk = NULL;
	}
	m_bConnect = TRUE;
	return hr;
}

HRESULT CaptureVideo::OpenPropertyWndBlocking()
{
	static bool bShowing = false;
	bool* pbShow = &bShowing;

	static std::thread* pThread = nullptr;
	auto ShowPropertyWnd = [this, pbShow] {
		*pbShow = true;
		ISpecifyPropertyPages* pSpec;
		CAUUID cauuid;
		HRESULT hr = this->m_pDevFilter->QueryInterface(IID_ISpecifyPropertyPages,
			(void**)&pSpec);
		if (hr == S_OK)
		{
			hr = pSpec->GetPages(&cauuid);

			hr = OleCreatePropertyFrame(::GetForegroundWindow(), 0, 0, NULL, 1,
				(IUnknown**)&(this->m_pDevFilter), cauuid.cElems,
				(GUID*)cauuid.pElems, 0, 0, NULL);

			CoTaskMemFree(cauuid.pElems);

			CCamProperty CamProperty;
			GetProperty(CamProperty);
			SaveProperty(this->m_CameraName,CamProperty);
		}	
		*pbShow = false;
	};
	if (!bShowing)
	{
		if (pThread && pThread->joinable())
		{
			pThread->join();
			delete pThread;
		}
		pThread = (new std::thread(ShowPropertyWnd));
	}
	return S_OK;
}

void CaptureVideo::GrabOneFrame(BOOL bGrab)
{
	g_sampleGrabberCB.m_bGetPicture = bGrab;
}

HRESULT CaptureVideo::HandleGraphCapturePicture(void)
{
	//////////////////////////////////////////////////////////////////////////
	HRESULT hr;
	long evCode = 0;
	long lBufferSize = 0;
	BYTE *p;
	hr = m_pMediaEvent->WaitForCompletion(INFINITE, &evCode); //
	if (SUCCEEDED(hr))
	{
		switch(evCode)
		{
		case EC_COMPLETE:
			m_pSampGrabber->GetCurrentBuffer(&lBufferSize, NULL);
			p = new BYTE[lBufferSize];
			m_pSampGrabber->GetCurrentBuffer(&lBufferSize, (LONG*)p);// get Current buffer
			g_sampleGrabberCB.SaveBitmap(p,lBufferSize); //save bitmap
			delete [] p;
			p = NULL;
			break;
		default:
			break;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	return hr;
}

#define SETCAMPROP(pProcAmp,PropName,Value)\
{\
	long Min, Max, Step, Default, Flag;\
	auto hresult = pProcAmp->GetRange(VideoProcAmp_##PropName, &Min, &Max, &Step, &Default, &Flag);\
	if (SUCCEEDED(hresult) && Value.##PropName != CCamProperty::NotValid)\
	{\
		pProcAmp->Set(VideoProcAmp_##PropName, Value.##PropName, Value.##PropName##Flag);\
	}\
}\

#define SETCAMControl(pControl,PropName,Value)\
{\
	long Min, Max, Step, Default, Flag;\
	auto hresult = pControl->GetRange(CameraControl_##PropName, &Min, &Max, &Step, &Default, &Flag);\
	if (SUCCEEDED(hresult) && Value.##PropName != CCamProperty::NotValid)\
	{\
		pControl->Set(CameraControl_##PropName, Value.##PropName, Value.##PropName##Flag);\
	}\
}\

void CaptureVideo::SetProperty(CCamProperty& CamProperty)
{
	IAMVideoProcAmp* pProcAmp;
	HRESULT hr = this->m_pDevFilter->QueryInterface(IID_IAMVideoProcAmp,
		(void**)&pProcAmp);
	if (SUCCEEDED(hr))
	{
		SETCAMPROP(pProcAmp, Brightness, CamProperty);
		SETCAMPROP(pProcAmp, Contrast, CamProperty);
		SETCAMPROP(pProcAmp, Hue, CamProperty);
		SETCAMPROP(pProcAmp, Saturation, CamProperty);
		SETCAMPROP(pProcAmp, Sharpness, CamProperty);
		SETCAMPROP(pProcAmp, Gamma, CamProperty);
		SETCAMPROP(pProcAmp, ColorEnable, CamProperty);
		SETCAMPROP(pProcAmp, WhiteBalance, CamProperty);
		SETCAMPROP(pProcAmp, BacklightCompensation, CamProperty);
		SETCAMPROP(pProcAmp, Gain, CamProperty);
	}

	IAMCameraControl* pControl;
	hr = this->m_pDevFilter->QueryInterface(IID_IAMCameraControl,
		(void**)&pControl);
	if (SUCCEEDED(hr))
	{
		SETCAMControl(pControl, Pan, CamProperty);
		SETCAMControl(pControl, Tilt, CamProperty);
		SETCAMControl(pControl, Roll, CamProperty);
		SETCAMControl(pControl, Zoom, CamProperty);
		SETCAMControl(pControl, Exposure, CamProperty);
		SETCAMControl(pControl, Iris, CamProperty);
		SETCAMControl(pControl, Focus, CamProperty);
	}
}


#define GETCAMPROP(pProcAmp,PropName,Value)\
{\
	long Min, Max, Step, Default, Flag;\
	auto hresult = pProcAmp->GetRange(VideoProcAmp_##PropName, &Min, &Max, &Step, &Default, &Flag);\
	if (SUCCEEDED(hresult))\
	{\
		long tempValue=CCamProperty::NotValid;\
		pProcAmp->Get(VideoProcAmp_##PropName, &tempValue, &Flag);\
		Value.##PropName=(int)tempValue;\
		Value.##PropName##Flag=(int)Flag;\
	}\
}\

#define GETCAMControl(pControl,PropName,Value)\
{\
	long Min, Max, Step, Default, Flag;\
	auto hresult = pControl->GetRange(CameraControl_##PropName, &Min, &Max, &Step, &Default, &Flag);\
	if (SUCCEEDED(hresult))\
	{\
		long tempValue=CCamProperty::NotValid;\
		pControl->Get(CameraControl_##PropName, &tempValue, &Flag);\
		Value.##PropName=(int)tempValue;\
		Value.##PropName##Flag=(int)Flag;\
	}\
}\

void CaptureVideo::GetProperty(CCamProperty& CamProperty)
{
	IAMVideoProcAmp* pProcAmp;
	HRESULT hr = this->m_pDevFilter->QueryInterface(IID_IAMVideoProcAmp,
		(void**)&pProcAmp);
	if (SUCCEEDED(hr))
	{
		GETCAMPROP(pProcAmp, Brightness, CamProperty);
		GETCAMPROP(pProcAmp, Contrast, CamProperty);
		GETCAMPROP(pProcAmp, Hue, CamProperty);
		GETCAMPROP(pProcAmp, Saturation, CamProperty);
		GETCAMPROP(pProcAmp, Sharpness, CamProperty);
		GETCAMPROP(pProcAmp, Gamma, CamProperty);
		GETCAMPROP(pProcAmp, ColorEnable, CamProperty);
		GETCAMPROP(pProcAmp, WhiteBalance, CamProperty);
		GETCAMPROP(pProcAmp, BacklightCompensation, CamProperty);
		GETCAMPROP(pProcAmp, Gain, CamProperty);
	}

	IAMCameraControl* pControl;
	hr = this->m_pDevFilter->QueryInterface(IID_IAMCameraControl,
		(void**)&pControl);
	if (SUCCEEDED(hr))
	{
		GETCAMControl(pControl, Pan, CamProperty);
		GETCAMControl(pControl, Tilt, CamProperty);
		GETCAMControl(pControl, Roll, CamProperty);
		GETCAMControl(pControl, Zoom, CamProperty);
		GETCAMControl(pControl, Exposure, CamProperty);
		GETCAMControl(pControl, Iris, CamProperty);
		GETCAMControl(pControl, Focus, CamProperty);
	}
}

bool CaptureVideo::ReadPropertyFromFile(std::wstring CameraName, CCamProperty& CamProperty)
{
	std::wstring Path = CaptureVideo::SaveDir + CameraName+ CaptureVideo::UID;
	std::ifstream ProFile(Path.c_str(),std::ios::binary|std::ios::in|std::ios::_Nocreate);
	if (ProFile.is_open())
	{
		int count = sizeof(CamProperty);
		char *Buffer=new char[count + sizeof(int) + 1];
		memset(Buffer, 0, count + sizeof(int) + 1);
		ProFile.read(Buffer, count + sizeof(int) + 1);
		if (*((int*)&(Buffer[0])) == count)
		{
			memcpy_s(&CamProperty, count, &Buffer[sizeof(int)], count);
			delete[] Buffer;
			return true;
		}
		delete[] Buffer;
	}
	return false;
}

bool CaptureVideo::SaveProperty(std::wstring CameraName, CCamProperty& CamProperty)
{
	std::wstring Path = CaptureVideo::SaveDir + CameraName+ CaptureVideo::UID;
	std::ofstream ProFile(Path.c_str());
	int count = sizeof(CamProperty);
	char* Buffer = new char[count + sizeof(int)+1];
	memset(Buffer, 0, count + sizeof(int) + 1);
	*((int*)&(Buffer[0])) = count;
	memcpy_s(&Buffer[sizeof(int)], count ,&CamProperty, count);
	ProFile.write(Buffer, count + sizeof(int) + 1);
	delete[] Buffer;
	return true;
}

GUID CaptureVideo::GetCapturePreferredMediaType()
{
	if (NULL == m_pCaptureGB)
	{
		return MEDIASUBTYPE_None;
	}
	int nCount(0);
	int nSize(0);
	IAMStreamConfig* pConfig(NULL);
	HRESULT hr = m_pCaptureGB->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video, m_pDevFilter,
		IID_IAMStreamConfig, (void**)&pConfig);
	if (FAILED(hr))
	{
		return MEDIASUBTYPE_None;
	}
	hr = pConfig->GetNumberOfCapabilities(&nCount, &nSize);
	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) != nSize)
	{
		return MEDIASUBTYPE_None;
	}
	VIDEO_STREAM_CONFIG_CAPS scc;
	AM_MEDIA_TYPE* pmt = NULL;
	std::vector<AM_MEDIA_TYPE*>	videoSupportMediaTypeList;
	for (int i = 0; i < nCount; i++)
	{
		pmt = NULL;
		hr = pConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&scc));
		if (FAILED(hr))
		{
			continue;
		}
		videoSupportMediaTypeList.push_back(pmt);
	}

	AM_MEDIA_TYPE* preferredAM = NULL;

	std::vector<std::pair<GUID, std::wstring> > mediaSubTypeList;
	
	bool bIsC950 = (m_CameraName.find(L"BCC950") != m_CameraName.npos);
	if (bIsC950)
	{
		mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_MJPG, L"MJPG"));
	}	
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_YUY2, L"YUY2"));
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_YV12, L"YV12"));	
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_I420, L"I420"));
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_RGB32, L"RGB32"));
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_RGB24, L"RGB24"));	
	mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_ARGB32, L"ARGB32"));
	if (!bIsC950)
	{
		mediaSubTypeList.push_back(std::make_pair(MEDIASUBTYPE_MJPG, L"MJPG"));
	}
	
	
	core::vec2i mCameraSrcSize;
	mCameraSrcSize.cx = 1920;
	mCameraSrcSize.cy = 1080;
	for (size_t index = 0; index < mediaSubTypeList.size(); ++index)
	{

		preferredAM = GetMediaTypeInList(videoSupportMediaTypeList,
			mCameraSrcSize.cx, mCameraSrcSize.cy, mediaSubTypeList[index].first);
		if (preferredAM)
		{
			GUID type = preferredAM->subtype;
			FreeMediaType(videoSupportMediaTypeList);
			return type;
		}
	}

	preferredAM = GetMediaTypeInList(videoSupportMediaTypeList,
		mCameraSrcSize.cx, 
		mCameraSrcSize.cy, 
		MAKEFOURCC('H', 'D', 'Y', 'C')
	);
	if(preferredAM)
	{
		GUID type = preferredAM->subtype;
		FreeMediaType(videoSupportMediaTypeList);
		return type;
	}
	FreeMediaType(videoSupportMediaTypeList);
	return MEDIASUBTYPE_RGB24;
}

GUID CaptureVideo::GetCapturePreferredMediaType2()
{
	bool bValid = false;
	GUID ResultGUID = MEDIASUBTYPE_RGB24;
	INT64 frameInterval = INT64(10000000.0);

	IAMStreamConfig* pSC;
	HRESULT hResult = m_pCaptureGB->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pDevFilter, IID_IAMStreamConfig, (void**)&pSC);

	if (S_OK != hResult || !pSC)
	{
		return MEDIASUBTYPE_RGB24;
	}

	int iCount = 0, iSize = 0;
	hResult = pSC->GetNumberOfCapabilities(&iCount, &iSize);
	if (S_OK != hResult)
	{
		return MEDIASUBTYPE_RGB24;
	}

	hResult = E_FAIL;
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		std::vector<AM_MEDIA_TYPE*> vecMediaTypes;
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmtConfig;
			HRESULT hr = pSC->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
			if (SUCCEEDED(hr))
			{
				if ((pmtConfig->majortype == MEDIATYPE_Video) &&
					(pmtConfig->formattype == FORMAT_VideoInfo) &&
					(pmtConfig->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
					(pmtConfig->pbFormat != NULL))
				{
					VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					LONG lWidth = pVih->bmiHeader.biWidth;
					LONG lHeight = pVih->bmiHeader.biHeight;

					if (!bValid)
					{
						ResultGUID = pmtConfig->subtype;
						bValid = true;
					}
					if (lWidth == 1920 && lHeight == 1080)
					{
						if (pVih->AvgTimePerFrame< frameInterval)
						{
							if (pmtConfig->subtype== MEDIASUBTYPE_MJPG||
								pmtConfig->subtype == MEDIASUBTYPE_YUY2 ||
								pmtConfig->subtype == MEDIASUBTYPE_YV12 ||
								pmtConfig->subtype == MEDIASUBTYPE_I420 ||
								pmtConfig->subtype == MEDIASUBTYPE_RGB32 ||
								pmtConfig->subtype == MEDIASUBTYPE_RGB24 ||
								pmtConfig->subtype == MEDIASUBTYPE_ARGB32 
								)
							{
								frameInterval = pVih->AvgTimePerFrame;
								ResultGUID = pmtConfig->subtype;
							}
						}
					}
				}
				StaticClassFunction::DeleteMediaType(pmtConfig);
			}
		}
	}
	return ResultGUID;
}

void CaptureVideo::FreeMediaType(std::vector<AM_MEDIA_TYPE*>& listType)
{
	std::vector<AM_MEDIA_TYPE*>::iterator it = listType.begin();
	for (; it != listType.end(); it++)
	{
		AM_MEDIA_TYPE* ptr = (*it);
		if (ptr) {
			StaticClassFunction::FreeMediaType(*ptr);
			ptr = NULL;
		}
	}
}


void CaptureVideo::SGReceiveDataCallBack(BYTE* pBuffer, long BufferLen, int width, int height)
{
	static UUID* hdycUUid=nullptr;
	if (!hdycUUid)
	{
		hdycUUid = new UUID;
		UuidFromStringA((RPC_CSTR)"43594448-0000-0010-8000-00AA00389B71", hdycUUid);
	}
	
	uint32_t format = 0;
	if (expectedMediaType == MEDIASUBTYPE_YV12)
	{
		format = libyuv::FOURCC_YV12;
	}
	else if (expectedMediaType == MEDIASUBTYPE_MJPG)
	{
		format = libyuv::FOURCC_MJPG;
	}
	else if (expectedMediaType == MEDIASUBTYPE_YUY2)
	{
		format = libyuv::FOURCC_YUY2;
	}
	else if (expectedMediaType == MEDIASUBTYPE_I420)
	{
		format = libyuv::FOURCC_I420;
	}
	else if (expectedMediaType == *hdycUUid)
	{
		format = libyuv::FOURCC_UYVY;
	}
	char* Data = (char*)malloc(width * height * 4);

	if (format != 0)
	{
		int ret = LibYUVWrapper::LibYUVConvertToARGB((unsigned char*)pBuffer, BufferLen, (uint8_t*)Data, width * 4,
			0, 0, width, height,
			width, height,
			libyuv::kRotate0, format);
	}
	else if (expectedMediaType == MEDIASUBTYPE_RGB24)
	{
		int srcPitch = width * 3;
		unsigned char* pSrcStart = (unsigned char*)pBuffer + (height - 1) * srcPitch;
		int rowPitch = width * 4;
		unsigned char* pDest = (unsigned char*)Data;
		for (int i = 0; i < height; i++)
		{
			unsigned char* pWrite = pDest;
			unsigned char* pRead = pSrcStart;
			for (int j = 0; j < width; j++)
			{
				*pWrite++ = *pRead++;
				*pWrite++ = *pRead++;
				*pWrite++ = *pRead++;
				*pWrite++ = 255;
			}

			pSrcStart = pSrcStart - srcPitch;
			pDest = pDest + rowPitch;
		}
	}
	else if (expectedMediaType == MEDIASUBTYPE_RGB32|| expectedMediaType == MEDIASUBTYPE_ARGB32)
	{		
		int srcPitch = width * 4;
		int targetPitch = width * 4;
		LPBYTE src = (LPBYTE)pBuffer + (height - 1) * srcPitch;
		LPBYTE dst = (LPBYTE)Data;
		{
			for (int i = 0; i < height; i++)
			{
				memcpy(dst, src, targetPitch);
				src -= srcPitch;
				dst += targetPitch;
			}
		}
	}
	else
	{
		memcpy_s(Data, BufferLen, pBuffer, BufferLen);
	}

	ReceiveDataCallBack(1, width, height, Data, nullptr,true);

}

static BITMAPINFOHEADER* GetVideoBMIHeader(const AM_MEDIA_TYPE* pMT)
{
	return (pMT->formattype == FORMAT_VideoInfo) ?
		&reinterpret_cast<VIDEOINFOHEADER*>(pMT->pbFormat)->bmiHeader :
		&reinterpret_cast<VIDEOINFOHEADER*>(pMT->pbFormat)->bmiHeader;
}

AM_MEDIA_TYPE* CaptureVideo::GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, DWORD fourCC)
{
	AM_MEDIA_TYPE* pAM = NULL;
	std::vector<AM_MEDIA_TYPE*>::iterator it = listType.begin();
	for (; it != listType.end(); it++)
	{
		VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)(*it)->pbFormat;
		BITMAPINFOHEADER* bmiHeader = GetVideoBMIHeader(*it);

		if (bmiHeader->biCompression == fourCC
			&& pvih->bmiHeader.biHeight == height
			&& pvih->bmiHeader.biWidth == width)
		{
			pAM = *it;
			break;
		}
	}
	return pAM;
}

AM_MEDIA_TYPE* CaptureVideo::GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, GUID subtype)
{
	AM_MEDIA_TYPE* pAM = NULL;
	std::vector<AM_MEDIA_TYPE*>::iterator it = listType.begin();
	for (; it != listType.end(); it++)
	{
		VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)(*it)->pbFormat;
		if ((*it)->subtype == subtype
			&& pvih->bmiHeader.biHeight == height
			&& pvih->bmiHeader.biWidth == width)
		{
			pAM = *it;
			break;
		}
	}
	return pAM;
}
