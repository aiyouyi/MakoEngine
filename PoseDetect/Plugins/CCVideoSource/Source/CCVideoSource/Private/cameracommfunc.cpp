
#include "cameracommfunc.h"

#pragma comment(lib,"strmbase.lib")

//#define SafeRelease(var) if(var) {var->Release(); var = NULL;}
const GUID PIN_CATEGORY_ROXIOCAPTURE = {0x6994AD05, 0x93EF, 0x11D0, {0xA3, 0xCC, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96}};

static void FreeMediaType(AM_MEDIA_TYPE &mt)
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

static void DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt != NULL)
	{
		FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}

HRESULT VACameraCommFunc::selectVideoDeviceByName( const std::wstring &strName,IBaseFilter **pOutFilter )
{
	HRESULT hr = S_OK;
	*pOutFilter = NULL;
	win32::com_ptr< ICreateDevEnum > pCreateDevEnum;
	hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_ALL, __uuidof(ICreateDevEnum), (void**)&pCreateDevEnum);

	if( !pCreateDevEnum || FAILED(hr))
	{
		return hr;
	}

	win32::com_ptr< IEnumMoniker > pEm;
	hr = pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );

	if( !pEm || FAILED(hr))
	{
		return hr;
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	win32::com_ptr< IMoniker > pM;
	hr = S_OK;

	while( (hr = pEm->Next( 1, &pM, &ulFetched )) == S_OK )
	{
		win32::com_ptr< IPropertyBag > pBag;
		hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
		if( hr != S_OK || !pBag)
		{
			continue;
		}

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read( L"FriendlyName", &devVar, NULL );
		if( hr != S_OK )
		{
			continue;
		}
		std::wstring friendlyName = devVar.bstrVal;
		::VariantClear(&devVar);
		if ( friendlyName == strName)
		{
			win32::com_ptr<IBaseFilter> pCap;
			hr = pM->BindToObject( 0, 0, IID_IBaseFilter, (void**)&pCap );
			*pOutFilter = pCap.detach();
			return S_OK;
		}
		pM.reset();
	}
	return E_FAIL;
}

HRESULT VACameraCommFunc::selectVideoDeviceByPath( const std::wstring &strName,IBaseFilter **pOutFilter )
{
	HRESULT hr = S_OK;
	*pOutFilter = NULL;
	win32::com_ptr< ICreateDevEnum > pCreateDevEnum;
	hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_ALL, __uuidof(ICreateDevEnum), (void**)&pCreateDevEnum);

	if( !pCreateDevEnum || FAILED(hr))
	{
		return hr;
	}

	win32::com_ptr< IEnumMoniker > pEm;
	hr = pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );

	if( !pEm || FAILED(hr))
	{
		return hr;
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	win32::com_ptr< IMoniker > pM;
	hr = S_OK;

	while( (hr = pEm->Next( 1, &pM, &ulFetched )) == S_OK )
	{
		win32::com_ptr< IPropertyBag > pBag;
		hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
		if( hr != S_OK || !pBag)
		{
			pM.reset();
			continue;
		}

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read( L"DevicePath", &devVar, NULL );
		if( hr != S_OK )
		{
			pM.reset();
			continue;
		}
		std::wstring devPathName = devVar.bstrVal;
		::VariantClear(&devVar);
		if ( devPathName == strName)
		{
			win32::com_ptr<IBaseFilter> pCap;
			hr = pM->BindToObject( 0, 0, IID_IBaseFilter, (void**)&pCap );
			*pOutFilter = pCap.detach();
			return S_OK;
		}
		pM.reset();
	}
	return E_FAIL;
}


HRESULT VACameraCommFunc::getCaptureResolution(ICaptureGraphBuilder2* pGraphicBuilder, IBaseFilter* deviceFilter, std::vector<core::vec2i> & videoRes)
{
	win32::com_ptr<IAMStreamConfig> pSC;

	HRESULT hr = pGraphicBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video, deviceFilter,
		IID_IAMStreamConfig, (void **)&pSC);

	if (S_OK != hr || !pSC)
	{
		return hr;
	}

	int iCount = 0, iSize = 0; 
	hr = pSC->GetNumberOfCapabilities(&iCount, &iSize); 

	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS) )
	{ 
		for (int iFormat = 0; iFormat < iCount; iFormat++) 
		{ 
			VIDEO_STREAM_CONFIG_CAPS scc; 
			AM_MEDIA_TYPE *pmtConfig; 
			hr = pSC->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc); 
			if (SUCCEEDED(hr)) 
			{ 
				if ((pmtConfig->majortype == MEDIATYPE_Video) &&
					(pmtConfig->formattype == FORMAT_VideoInfo) &&
					(pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
					(pmtConfig->pbFormat != NULL)) 
				{ 
					VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
					LONG lWidth = pVih->bmiHeader.biWidth; 
					LONG lHeight = pVih->bmiHeader.biHeight; 
					core::vec2i resolution;
					resolution.cx = lWidth;
					resolution.cy = lHeight;
					videoRes.push_back(resolution);
				}
				DeleteMediaType(pmtConfig); 
			}
		}
	}
	return hr;
}


std::wstring VACameraCommFunc::getDeviceFriendlyNameByDevicePath( const std::wstring &devicePath )
{
	HRESULT hr = S_OK;
	win32::com_ptr< ICreateDevEnum > pCreateDevEnum;
	hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_ALL, __uuidof(ICreateDevEnum), (void**)&pCreateDevEnum);

	if( !pCreateDevEnum || FAILED(hr))
	{
		return L"";
	}

	win32::com_ptr< IEnumMoniker > pEm;
	pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );

	if( !pEm || FAILED(hr))
	{
		return L"";
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	win32::com_ptr< IMoniker > pM;
	hr = S_OK;

	while( (hr = pEm->Next( 1, &pM, &ulFetched )) == S_OK )
	{
		win32::com_ptr< IPropertyBag > pBag;
		hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
		if( hr != S_OK || !pBag)
		{
			continue;
		}

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read(L"DevicePath", &devVar, 0);
		if( hr != S_OK )
		{
			continue;
		}
		std::wstring keydevicePath = devVar.bstrVal;
		::VariantClear(&devVar);

		if (devicePath == keydevicePath)
		{
			std::wstring friendlyName;
			VARIANT var;
			::VariantInit(&var);
			var.vt = VT_BSTR;
			hr = pBag->Read( L"FriendlyName", &var, NULL );
			if( hr == S_OK )
			{
				friendlyName = var.bstrVal;
				::VariantClear(&var);
			}
			return friendlyName;
		}		
		pM.reset();
	}

	return L"";
}

std::wstring VACameraCommFunc::getPathByFriendlyName(const std::wstring &friendlyName)
{
	HRESULT hr = S_OK;	
	win32::com_ptr< ICreateDevEnum > pCreateDevEnum;
	hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_ALL, __uuidof(ICreateDevEnum), (void**)&pCreateDevEnum);

	if( !pCreateDevEnum || FAILED(hr))
	{
		return L"";
	}

	win32::com_ptr< IEnumMoniker > pEm;
	pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );

	if( !pEm || FAILED(hr))
	{
		return L"";
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	win32::com_ptr< IMoniker > pM;
	hr = S_OK;

	while( (hr = pEm->Next( 1, &pM, &ulFetched )) == S_OK )
	{
		win32::com_ptr< IPropertyBag > pBag;
		hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
		if( hr != S_OK || !pBag)
		{
			continue;
		}

		VARIANT varName;
		varName.vt = VT_BSTR;
		::VariantInit(&varName);
		hr = pBag->Read(L"FriendlyName", &varName, 0);
		if( hr != S_OK )
		{
			continue;
		}
		std::wstring keyfriendlyName = varName.bstrVal;
		::VariantClear(&varName);

		if (friendlyName == keyfriendlyName)
		{
			std::wstring devicePath;
			VARIANT var;
			::VariantInit(&var);
			var.vt = VT_BSTR;
			hr = pBag->Read( L"DevicePath", &var, NULL );
			if( hr == S_OK )
			{
				devicePath = var.bstrVal;
				::VariantClear(&var);
			}
			return devicePath;
		}
		pM.reset();
	}

	return L"";
}

HRESULT VACameraCommFunc::getDeviceList(std::vector<std::wstring> &vecDevices)
{
    CoInitialize(NULL);

	HRESULT hr = S_OK;
	win32::com_ptr< ICreateDevEnum > pCreateDevEnum;
	hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_ALL, __uuidof(ICreateDevEnum), (void**)&pCreateDevEnum);

	if( !pCreateDevEnum || FAILED(hr))
	{
		return hr;
	}

	win32::com_ptr< IEnumMoniker > pEm;
	pCreateDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEm, 0 );

	if( !pEm || FAILED(hr))
	{
		return hr;
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	win32::com_ptr< IMoniker > pM;
	hr = S_OK;

	while( (hr = pEm->Next( 1, &pM, &ulFetched )) == S_OK )
	{

		win32::com_ptr< IPropertyBag > pBag;
		hr = pM->BindToStorage( 0, 0, IID_IPropertyBag, (void**) &pBag );
		if( hr != S_OK || !pBag)
		{
			continue;
		}

		VARIANT var;
		::VariantInit(&var);
		var.vt = VT_BSTR;
		hr = pBag->Read( L"FriendlyName", &var, NULL );
		if( hr != S_OK )
		{
			continue;
		}
		::VariantClear(&var);
		vecDevices.push_back(var.bstrVal);

		pM.reset();
	}

	return S_OK;
}

HRESULT VACameraCommFunc::getDevicePathList(std::vector<VACameraDeviceInfo> &vecDevices)
{
	CoInitialize(NULL);

	HRESULT hr = S_OK;
	win32::com_ptr< ICreateDevEnum > pCreateDevEnum;
	hr = ::CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_ALL, __uuidof(ICreateDevEnum), (void**)&pCreateDevEnum);

	if (!pCreateDevEnum || FAILED(hr))
	{
		return hr;
	}

	win32::com_ptr< IEnumMoniker > pEm;
	pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);

	if (!pEm || FAILED(hr))
	{
		return hr;
	}

	pEm->Reset();

	ULONG ulFetched = 0;
	win32::com_ptr< IMoniker > pM;
	hr = S_OK;

	while ((hr = pEm->Next(1, &pM, &ulFetched)) == S_OK)
	{
		win32::com_ptr< IPropertyBag > pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pBag);
		if (hr != S_OK || !pBag)
		{
			continue;
		}
		VACameraDeviceInfo devInfo;
		VARIANT friendlyName;
		::VariantInit(&friendlyName);
		friendlyName.vt = VT_BSTR;
		hr = pBag->Read(L"FriendlyName", &friendlyName, NULL);
		if (hr != S_OK)
		{
			continue;
		}
		devInfo.friendlyName = friendlyName.bstrVal;
		::VariantClear(&friendlyName);

		VARIANT devVar;
		::VariantInit(&devVar);
		devVar.vt = VT_BSTR;
		hr = pBag->Read(L"DevicePath", &devVar, 0);
		if (hr == S_OK)
		{
			devInfo.devicePath = devVar.bstrVal;
			devInfo.IsCamreaTool = false;
			::VariantClear(&devVar);
		}
		else
		{
			devInfo.devicePath = devInfo.friendlyName;
			devInfo.IsCamreaTool = true;
		}
		vecDevices.push_back(devInfo);
		pM.reset();
	}
	return S_OK;
}