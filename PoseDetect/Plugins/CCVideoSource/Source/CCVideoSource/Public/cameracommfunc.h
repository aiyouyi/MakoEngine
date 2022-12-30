#pragma once
//#include "core/inc.h"
//#include "win/win32.h"
//#include "core/vec2.h"

//#define __STREAMS__
#include "CoreMinimal.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <dshow.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include <Amaudio.h>
#include <Dvdmedia.h>
#include "win/com_ptr.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include <vector>
#include <string>

//extern void FreeMediaType(AM_MEDIA_TYPE& mt);
//extern HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource);
//
//extern void DeleteMediaType(AM_MEDIA_TYPE *pmt);

// Platform code uses IsMaximized which is defined to IsZoomed by windowsx.h
#undef IsMaximized
#undef IsMinimized

class MediaTypePtr {
	friend class MediaType;

	AM_MEDIA_TYPE *ptr;

public:
	inline void Clear()
	{
		if (ptr) {
			//FreeMediaType(*ptr);
			//CoTaskMemFree(ptr);
			ptr = NULL;
		}
	}

	inline MediaTypePtr() : ptr(NULL) {}
	inline MediaTypePtr(AM_MEDIA_TYPE *ptr_) : ptr(ptr_) {}
	inline ~MediaTypePtr() {Clear();}

	inline AM_MEDIA_TYPE **operator&() {Clear(); return &ptr;}

	inline AM_MEDIA_TYPE *operator->() const {return ptr;}

	inline operator AM_MEDIA_TYPE*() const {return ptr;}

	inline void operator=(AM_MEDIA_TYPE *ptr_) {Clear(); ptr = ptr_;}

	inline bool operator==(const AM_MEDIA_TYPE *ptr_) const
	{
		return ptr == ptr_;
	}
};

struct VACameraDeviceInfo
{
	std::wstring friendlyName;
	std::wstring devicePath;
	bool IsCamreaTool;

	VACameraDeviceInfo()
		:IsCamreaTool(false)
	{

	}
};

class CCVIDEOSOURCE_API VACameraCommFunc
{
public:
	static HRESULT selectVideoDeviceByName( const std::wstring &strName,IBaseFilter **pOutFilter);
	static HRESULT selectVideoDeviceByPath( const std::wstring &strName,IBaseFilter **pOutFilter);
	static HRESULT getCaptureResolution(ICaptureGraphBuilder2* pGraphicBuilder, IBaseFilter* deviceFilter, std::vector<core::vec2i> & videoRes);
	static HRESULT setCaptureResolution(win32::com_ptr<ICaptureGraphBuilder2> pGraphicBuilder, win32::com_ptr<IBaseFilter> deviceFilter, core::vec2i resolution,UINT64 frameInterval ,GUID expectedMediaType);
	static std::wstring getDeviceFriendlyNameByDevicePath(const std::wstring &devicePath);
	static std::wstring getPathByFriendlyName(const std::wstring &friendlyName);
	static HRESULT getDeviceList(std::vector<std::wstring> &vecDevices);
	static HRESULT getDevicePathList(std::vector<VACameraDeviceInfo> &vecDevices);
	static IPin* GetOutputPin(IBaseFilter *filter, const GUID *majorType);
};
