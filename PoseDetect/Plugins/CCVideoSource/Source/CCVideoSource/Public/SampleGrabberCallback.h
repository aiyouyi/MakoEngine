#ifndef __SAMPLEGRABBERCALLBACK_H__
#define __SAMPLEGRABBERCALLBACK_H__
#include "common.h"
#include "core/event.h"


class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	//typedef void (*ReceiveDataCallBack)(BYTE* pBuffer, long BufferLen ,int width, int height, void* pCaptureContext);

	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void** ppvObject);

	HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample *pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE *pBuffer, long BufferLen);
	
	SampleGrabberCallback();
	virtual ~SampleGrabberCallback() {}
	BOOL SaveBitmap(BYTE * pBuffer, long lBufferSize ); //±£´æbitmapÍ¼Æ¬
public:
	BOOL m_bGetPicture;  // is get a picture
	long m_lWidth;       //´æ´¢Í¼Æ¬µÄ¿í¶È
	long m_lHeight;		 //´æ´¢Í¼Æ¬µÄ³¤¶È
	int  m_iBitCount;    //the number of bits per pixel (bpp)
	TCHAR m_chTempPath[MAX_PATH];
	TCHAR m_chSwapStr[MAX_PATH];
	TCHAR m_chDirName[MAX_PATH];

	core::event<void(BYTE* pBuffer, long BufferLen, int width, int height)> ReceiveDataCallBack;

};

#endif //__SAMPLEGRABBERCALLBACK_H__