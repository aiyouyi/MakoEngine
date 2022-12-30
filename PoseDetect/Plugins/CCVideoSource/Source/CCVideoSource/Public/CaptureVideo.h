#ifndef __CAPTUREVIDEO_H__
#define __CAPTUREVIDEO_H__
#include "SampleGrabberCallback.h"



class CaptureVideo
{
public:

	static std::wstring SaveDir;
	static std::wstring UID;

	CaptureVideo();
	~CaptureVideo();
	HRESULT InitializeEnv(); //initialize environment
	HRESULT EnumAllDevices(HWND hCombox);  //Enumeration all devices
	void CloseInterface(); //close all interface
	HRESULT OpenDevice(const std::wstring& deviceName);
	HRESULT OpenPropertyWndBlocking();
	HRESULT BindFilter(int deviceID, IBaseFilter** pBaseFilter);
	HRESULT SetupVideoWindow(LONG nLeft, LONG nTop, LONG nWidth, LONG nHeight);
	void ResizeVideoWindow(LONG nLeft, LONG nTop, LONG nWidth, LONG nHeight);
	void GrabOneFrame(BOOL bGrab); // use ISampleGrabberCallbackCB get a picture
	HRESULT HandleGraphCapturePicture(void); //capture picture  use GetCurrentBuffer

	struct CCamProperty
	{
		static const int NotValid = 0xFFFFFFFF;
		int Brightness;
		int Contrast;
		int Hue;
		int Saturation;
		int Sharpness;
		int Gamma;
		int ColorEnable;
		int WhiteBalance;
		int BacklightCompensation;
		int Gain;

		int Pan;
		int Tilt;
		int Roll;
		int Zoom;
		int Exposure;
		int Iris;
		int Focus;


		int BrightnessFlag;
		int ContrastFlag;
		int HueFlag;
		int SaturationFlag;
		int SharpnessFlag;
		int GammaFlag;
		int ColorEnableFlag;
		int WhiteBalanceFlag;
		int BacklightCompensationFlag;
		int GainFlag;

		int PanFlag;
		int TiltFlag;
		int RollFlag;
		int ZoomFlag;
		int ExposureFlag;
		int IrisFlag;
		int FocusFlag;
	};
	void SetProperty(CCamProperty& CamProperty);
	void GetProperty(CCamProperty& CamProperty);
	bool ReadPropertyFromFile(std::wstring CameraName, CCamProperty& CamProperty);
	bool SaveProperty(std::wstring CameraName, CCamProperty& CamProperty);

	HRESULT selectVideoDeviceByName(const std::wstring& deviceName, IBaseFilter** pOutFilter);

	GUID GetCapturePreferredMediaType();
	GUID GetCapturePreferredMediaType2();
	AM_MEDIA_TYPE* GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, DWORD fourCC);
	AM_MEDIA_TYPE* GetMediaTypeInList(std::vector<AM_MEDIA_TYPE*>& listType, long width, long height, GUID subtype);

	void FreeMediaType(std::vector<AM_MEDIA_TYPE*>& listType);
	void SGReceiveDataCallBack(BYTE* pBuffer, long BufferLen, int width, int height);

	core::event<void(int Result, unsigned int Width, unsigned int Height, char* Data, const char* ErrorStr, bool fromlocalCamera)> ReceiveDataCallBack;
	GUID expectedMediaType;

	std::wstring GetCameraName() const { return m_CameraName; }

private:
	IGraphBuilder* m_pGraphBuilder;
	ICaptureGraphBuilder2* m_pCaptureGB;
	IMediaControl* m_pMediaControl;
	IBaseFilter* m_pDevFilter;
	ISampleGrabber* m_pSampGrabber;
	IMediaEventEx* m_pMediaEvent;
	std::wstring m_CameraName;
public:
	BOOL m_bConnect;

};


#endif  //__CAPTUREVIDEO_H__