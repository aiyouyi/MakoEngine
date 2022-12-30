#include "win/mediafoundation/capture.h"
#include "win/win32.h"
#include "core/logger.h"

#pragma warning (disable : 4668) 
#define MF_INIT_GUIDS
#include <shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#undef MF_INIT_GUIDS
#include "core/temp_ptr.h"
#include "win/com_ptr.h"
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Shlwapi.lib")

struct pixel_format_pair
{
    core::pixel_format format;
    GUID guid;
};

static const pixel_format_pair pixel_format_pairs[] =
{
    { core::pixel_format::i420, MFVideoFormat_I420 },
    { core::pixel_format::i420, MFVideoFormat_IYUV },
    { core::pixel_format::xrgb, MFVideoFormat_RGB32 },
    { core::pixel_format::rgb, MFVideoFormat_RGB24 },
    { core::pixel_format::yuy2, MFVideoFormat_YUY2 },
    { core::pixel_format::yvyu, MFVideoFormat_YVYU },
    { core::pixel_format::uyvy, MFVideoFormat_UYVY },
    { core::pixel_format::nv12, MFVideoFormat_NV12 },
    { core::pixel_format::mjpg, MFVideoFormat_MJPG },
    { core::pixel_format::h264, MFVideoFormat_H264 },
#if defined WDK_NTDDI_VERSION && (WDK_NTDDI_VERSION >= NTDDI_WIN10)
    { core::pixel_format::h265, MFVideoFormat_H265 },
#endif
    { core::pixel_format::hevc, MFVideoFormat_HEVC },
};

struct sample_format_pair
{
    core::sample_format format;
    GUID guid;
};

static const sample_format_pair sample_format_pairs[] =
{
    { core::sample_format::float32, MFAudioFormat_Float },
#if defined WDK_NTDDI_VERSION && (WDK_NTDDI_VERSION >= NTDDI_WIN10)
#endif
};

static core::pixel_format pixel_format_from_GUID(const GUID & guid)
{
    for(auto & pair : pixel_format_pairs)
    {
        if (pair.guid == guid)
            return pair.format;
    }
    return core::pixel_format::none;
}

static core::sample_format sample_format_from_GUID(const GUID & guid)
{
    for (auto & pair : sample_format_pairs)
    {
        if (pair.guid == guid)
            return pair.format;
    }
    return core::sample_format::none;
}

static const GUID & pixel_format_to_GUID(core::pixel_format format)
{
    for (auto & pair : pixel_format_pairs)
    {
        if (pair.format == format)
            return pair.guid;
    }
    return MFVideoFormat_Base;
}

static const GUID & sample_format_to_GUID(core::sample_format format)
{
    for (auto & pair : sample_format_pairs)
    {
        if (pair.format == format)
            return pair.guid;
    }
    return MFAudioFormat_Base;
}

class video_capture_source_impl : public video_capture_source, public IMFSourceReaderCallback
{
public:
    video_capture_source_impl(win32::com_ptr<IMFMediaSource> source) : _source(source) {}
    ~video_capture_source_impl()
    {
        _reader.reset();
        _source.reset();
    }
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv) override
    {
        static const QITAB qit[] =
        {
            QITABENT(video_capture_source_impl, IMFSourceReaderCallback),
        { 0 },
        };
        return QISearch(this, qit, iid, ppv);
    }


    STDMETHODIMP_(ULONG) AddRef() override
    {
        return _InterlockedIncrement(&_ref);
    }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG uCount = _InterlockedDecrement(&_ref);
        if (uCount == 0)
        {
            delete this;
        }
        // For thread safety, return a temporary variable.
        return uCount;
    }


    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(
        HRESULT hrStatus,
        DWORD dwStreamIndex,
        DWORD dwStreamFlags,
        LONGLONG llTimestamp,
        IMFSample *pSample
    ) override
    {
        if (FAILED(hrStatus))
        {
            core::logger::war() << __FUNCTION__ " OnReadSample failed, " << win32::winerr_str(hrStatus);
            return hrStatus;
        }

        HRESULT hr = S_OK;
        // 这是可以的
        if (pSample)
        {
            {
                std::lock_guard<std::mutex> l(_mtx);
                if (!_reader)
                    return S_OK;
            }

			win32::com_ptr<IMFMediaBuffer> buffer;
            hr = pSample->GetBufferByIndex(0, buffer.getpp());
            if (FAILED(hr))
            {
                core::logger::war() << __FUNCTION__ " GetBufferByIndex failed, " << win32::winerr_str(hr);
                return hr;
            }

            byte_t * data = nullptr;
            int32_t stride = 0;
			win32::com_ptr<IMF2DBuffer> buffer2d = buffer;
            if (buffer2d)
            {
                hr = buffer2d->Lock2D(reinterpret_cast<BYTE **>(&data), (LONG *)&stride);
                if (FAILED(hr))
                {
                    core::logger::war() << __FUNCTION__ " Lock2D failed, " << win32::winerr_str(hr);
                    return hr;
                }
            }
            else
            {
                hr = buffer->Lock(reinterpret_cast<BYTE **>(&data), nullptr, nullptr);
                if (FAILED(hr))
                {
                    core::logger::war() << __FUNCTION__ " Lock failed, " << win32::winerr_str(hr);
                    return hr;
                }

                if (_default_stride < 0 && _mode.size.cy > 0)
                    data = data + std::abs(_default_stride) * (_mode.size.cy - 1);
                stride = _default_stride;
            }

            sampled(data, stride, _mode);
            if(buffer2d)
            {
                hr = buffer2d->Unlock2D();
                if (FAILED(hr))
                {
                    core::logger::war() << __FUNCTION__ " Unlock2D failed, " << win32::winerr_str(hr);
                    return hr;
                }
            }
            else
            {
                hr = buffer->Unlock();
                if (FAILED(hr))
                {
                    core::logger::war() << __FUNCTION__ " Unlock failed, " << win32::winerr_str(hr);
                    return hr;
                }
            }
        }

        {
            std::lock_guard<std::mutex> l(_mtx);
            if(_reader)
            {
                hr = _reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
                if (FAILED(hr))
                {
                    core::logger::war() << __FUNCTION__ " ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, " << win32::winerr_str(hr);
                    return S_OK;
                }
            }
        }
        return S_OK;
    }

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) override
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD) override
    {
        return S_OK;
    }

public:
    core::error_e start(const video_capture_mode & mode) override
    {
        std::lock_guard<std::mutex> l(_mtx);
        if (_reader)
            return core::error_ok;

        if (!_source)
            return core::error_state;

        HRESULT hr = S_OK;

		win32::com_ptr<IMFAttributes> reader_attributes;
        hr = MFCreateAttributes(reader_attributes.getpp(), 2);
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " MFCreateAttributes failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = reader_attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true);
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " set MF_READWRITE_DISABLE_CONVERTERS failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }
        hr = reader_attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " set MF_READWRITE_DISABLE_CONVERTERS failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

		win32::com_ptr<IMFSourceReader> reader;
        hr = MFCreateSourceReaderFromMediaSource(_source.get(), reader_attributes.get(), reader.getpp());
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " MFCreateSourceReaderFromMediaSource failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

		win32::com_ptr<IMFMediaType> type;
        hr = MFCreateMediaType(type.getpp());
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFCreateMediaType failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetGUID(MF_MT_MAJOR_TYPE) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        auto format = pixel_format_to_GUID(mode.formmat);
        if (format == MFVideoFormat_Base)
        {
            core::logger::war() << __FUNCTION__ " invalid format";
            return core::error_bad_format;
        }

        hr = type->SetGUID(MF_MT_SUBTYPE, format);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetGUID(MF_MT_SUBTYPE) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = MFSetAttributeSize(type.get(), MF_MT_FRAME_SIZE, mode.size.cx, mode.size.cy);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFSetAttributeSize(MF_MT_FRAME_SIZE) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = MFSetAttributeRatio(type.get(), MF_MT_FRAME_RATE, mode.framerate.num, mode.framerate.den);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFSetAttributeRatio(MF_MT_FRAME_RATE) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type.get());
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        int32_t stride = 0;
        hr = type->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32 *)&stride);
        if (FAILED(hr))
        {
            hr = MFGetStrideForBitmapInfoHeader(format.Data1, mode.size.cx, (LONG *)&stride);
            if (FAILED(hr))
            {
                core::logger::war() << __FUNCTION__ " MFGetStrideForBitmapInfoHeader failed, " << win32::winerr_str(hr);
                return win32::winerr_val(hr);
            }
        }

        _reader = reader;
        _mode = mode;
        _default_stride = stride;
        return core::error_ok;
    }

    core::error_e stop() override
    {
        std::lock_guard<std::mutex> l(_mtx);
        if (!_source)
            return core::error_ok;

        _reader.reset();
        if (_source)
            _source->Shutdown();
        _source.reset();
        return core::error_ok;
    }

	const video_capture_mode& currentMode() override
	{
		std::lock_guard<std::mutex> l(_mtx);
		return _mode;
	}

private:
    long _ref = 1;
    std::mutex _mtx;
	win32::com_ptr<IMFMediaSource> _source;
	win32::com_ptr<IMFSourceReader> _reader;
    video_capture_mode _mode;
    int32_t _default_stride = 0;
};

class audio_capture_source_impl : public audio_capture_source, public IMFSourceReaderCallback
{
public:
    audio_capture_source_impl(win32::com_ptr<IMFMediaSource> source) : _source(source) {}
    ~audio_capture_source_impl()
    {
        _reader.reset();
        _source.reset();
    }
    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void ** ppv) override
    {
        static const QITAB qit[] =
        {
            QITABENT(audio_capture_source_impl, IMFSourceReaderCallback),
        { 0 },
        };
        return QISearch(this, qit, iid, ppv);
    }


    STDMETHODIMP_(ULONG) AddRef() override
    {
        return _InterlockedIncrement(&_ref);
    }
    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG uCount = _InterlockedDecrement(&_ref);
        if (uCount == 0)
        {
            delete this;
        }
        // For thread safety, return a temporary variable.
        return uCount;
    }


    // IMFSourceReaderCallback methods
    STDMETHODIMP OnReadSample(
        HRESULT hrStatus,
        DWORD dwStreamIndex,
        DWORD dwStreamFlags,
        LONGLONG llTimestamp,
        IMFSample * pSample
    ) override
    {
        if (FAILED(hrStatus))
        {
            core::logger::war() << __FUNCTION__ " OnReadSample failed, " << win32::winerr_str(hrStatus);
            return hrStatus;
        }

        HRESULT hr = S_OK;
        // 这是可以的
        if (pSample)
        {
            {
                std::lock_guard<std::mutex> l(_mtx);
                if (!_reader)
                    return S_OK;
            }

			win32::com_ptr<IMFMediaBuffer> buffer;
            hr = pSample->GetBufferByIndex(0, buffer.getpp());
            if (FAILED(hr))
            {
                core::logger::war() << __FUNCTION__ " GetBufferByIndex failed, " << win32::winerr_str(hr);
                return hr;
            }

            byte_t * data = nullptr;
            hr = buffer->Lock(reinterpret_cast<BYTE * *>(&data), nullptr, nullptr);
            if (FAILED(hr))
            {
                core::logger::war() << __FUNCTION__ " Lock failed, " << win32::winerr_str(hr);
                return hr;
            }

            DWORD dwLength = 0;
            hr = buffer->GetCurrentLength(&dwLength);
            if (FAILED(hr))
            {
                core::logger::war() << __FUNCTION__ " GetCurrentLength failed, " << win32::winerr_str(hr);
                hr = buffer->Unlock();
                return hr;
            }

            sampled(data, dwLength, _mode);
            hr = buffer->Unlock();
            if (FAILED(hr))
            {
                core::logger::war() << __FUNCTION__ " Unlock failed, " << win32::winerr_str(hr);
                return hr;
            }
        }

        {
            std::lock_guard<std::mutex> l(_mtx);
            if (_reader)
            {
                hr = _reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, NULL, NULL, NULL);
                if (FAILED(hr))
                {
                    core::logger::war() << __FUNCTION__ " ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, " << win32::winerr_str(hr);
                    return S_OK;
                }
            }
        }
        return S_OK;
    }

    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *) override
    {
        return S_OK;
    }

    STDMETHODIMP OnFlush(DWORD) override
    {
        return S_OK;
    }

public:
    core::error_e start(const audio_capture_mode & mode) override
    {
        std::lock_guard<std::mutex> l(_mtx);
        if (_reader)
            return core::error_ok;

        if (!_source)
            return core::error_state;

        HRESULT hr = S_OK;

		win32::com_ptr<IMFAttributes> reader_attributes;
        hr = MFCreateAttributes(reader_attributes.getpp(), 2);
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " MFCreateAttributes failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = reader_attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true);
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " set MF_READWRITE_DISABLE_CONVERTERS failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }
        hr = reader_attributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " set MF_READWRITE_DISABLE_CONVERTERS failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

		win32::com_ptr<IMFSourceReader> reader;
        hr = MFCreateSourceReaderFromMediaSource(_source.get(), reader_attributes.get(), reader.getpp());
        if (FAILED(hr))
        {
            core::logger::err() << __FUNCTION__ " MFCreateSourceReaderFromMediaSource failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

		win32::com_ptr<IMFMediaType> type;
        hr = MFCreateMediaType(type.getpp());
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFCreateMediaType failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetGUID(MF_MT_MAJOR_TYPE) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        auto format = sample_format_to_GUID(mode.format);
        if (format == MFAudioFormat_Base)
        {
            core::logger::war() << __FUNCTION__ " invalid format";
            return core::error_bad_format;
        }

        hr = type->SetGUID(MF_MT_SUBTYPE, format);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetGUID(MF_MT_SUBTYPE) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, mode.channels);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetUINT32(MF_MT_AUDIO_NUM_CHANNELS) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, mode.sample_rate);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, type.get());
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        hr = reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, NULL, NULL, NULL);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM) failed, " << win32::winerr_str(hr);
            return win32::winerr_val(hr);
        }

        _reader = reader;
        _mode = mode;
        return core::error_ok;
    }

    core::error_e stop() override
    {
        std::lock_guard<std::mutex> l(_mtx);
        if (!_source)
            return core::error_ok;

        _reader.reset();
        if (_source)
            _source->Shutdown();
        _source.reset();
        return core::error_ok;
    }

    const audio_capture_mode & currentMode() override
    {
        std::lock_guard<std::mutex> l(_mtx);
        return _mode;
    }

private:
    long _ref = 1;
    std::mutex _mtx;
	win32::com_ptr<IMFMediaSource> _source;
	win32::com_ptr<IMFSourceReader> _reader;
    audio_capture_mode _mode;
};

std::vector<video_capture_device_desc> video_devices()
{
    HRESULT hr = S_OK;
	win32::com_ptr<IMFAttributes> enumerator_attributes;
    hr = MFCreateAttributes(enumerator_attributes.getpp(), 1);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = enumerator_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        return {};
    }

    IMFActivate ** activates = nullptr;
    UINT32 count = 0;
    std::unique_ptr<void, std::function<void(void*)>> devices_releaser(nullptr, [&](void*)
    {
        if (activates)
        {
            for (UINT32 i = 0; i < count; i++)
                SafeRelease(&activates[i]);
            CoTaskMemFree(activates);
        }
    });

    //std::unique_ptr<void, decltype(void())*> devices_releaser(nullptr, [](){});

    hr = MFEnumDeviceSources(enumerator_attributes.get(), &activates, &count);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFEnumDeviceSources failed, " << win32::winerr_str(hr);
        return {};
    }

    std::vector<video_capture_device_desc> devices;
    for(UINT32 idevice = 0; idevice < count; ++idevice)
    {
        IMFActivate * device = activates[idevice];
        core::temp_ptr<WCHAR, decltype(CoTaskMemFree) *> friendlyName(nullptr, CoTaskMemFree);
        hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, friendlyName.getpp(), NULL);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME) failed at " << idevice << ", "<< win32::winerr_str(hr);
            continue;
        }

        core::temp_ptr<WCHAR, decltype(CoTaskMemFree) *> deviceId(nullptr, CoTaskMemFree);
        hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, deviceId.getpp(), NULL);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK) failed at " << idevice << ", " << win32::winerr_str(hr);
            continue;
        }

        // 部分设备（BlackMagic 采集卡）MFCreateDeviceSource 可能会失败，这里可以过滤掉
        //{
        //    com_ptr<IMFAttributes> source_attributes;
        //    hr = MFCreateAttributes(source_attributes.getpp(), 2);
        //    if (FAILED(hr))
        //    {
        //        core::logger::war() << __FUNCTION__ " set MFCreateAttributes failed, " << win32::winerr_str(hr);
        //        continue;
        //    }

        //    hr = source_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        //    if (FAILED(hr))
        //    {
        //        core::logger::war() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        //        continue;
        //    }

        //    hr = source_attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, deviceId.get());
        //    if (FAILED(hr))
        //    {
        //        core::logger::war() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK failed, " << win32::winerr_str(hr);
        //        continue;
        //    }

        //    com_ptr<IMFMediaSource> source;
        //    hr = MFCreateDeviceSource(source_attributes.get(), source.getpp());
        //    if (FAILED(hr))
        //    {
        //        core::logger::err() << __FUNCTION__ " MFCreateDeviceSource failed, " << win32::winerr_str(hr);
        //        continue;
        //    }
        //}

        devices.push_back({ core::ucs2_u8(friendlyName.get()), core::ucs2_u8(deviceId.get()) });
    }

    return devices;
}

std::vector<video_capture_mode> video_device_modes(std::string deviceid)
{
    HRESULT hr = S_OK;

	win32::com_ptr<IMFAttributes> source_attributes;
    hr = MFCreateAttributes(source_attributes.getpp(), 2);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, core::u8_ucs2(deviceid).c_str());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK failed, " << win32::winerr_str(hr);
        return {};
    }

	win32::com_ptr<IMFMediaSource> source;
    hr = MFCreateDeviceSource(source_attributes.get(), source.getpp());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateDeviceSource failed, " << win32::winerr_str(hr);
        return {};
    }

	win32::com_ptr<IMFAttributes> reader_attributes;
    hr = MFCreateAttributes(reader_attributes.getpp(), 1);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = reader_attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_READWRITE_DISABLE_CONVERTERS failed, " << win32::winerr_str(hr);
        return {};
    }
	win32::com_ptr<IMFSourceReader> reader;
    hr = MFCreateSourceReaderFromMediaSource(source.get(), reader_attributes.get(), reader.getpp());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateSourceReaderFromMediaSource failed, " << win32::winerr_str(hr);
        return {};
    }

    std::vector<video_capture_mode> modes;
    uint32_t type_index = 0;
    while(true)
    {
		win32::com_ptr<IMFMediaType> type;
        hr = reader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, type_index++, type.getpp());
        if (FAILED(hr))
            break;

        GUID format = { };
        hr = type->GetGUID(MF_MT_SUBTYPE, &format);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " get MF_MT_SUBTYPE failed, " << win32::winerr_str(hr);
            continue;
        }

        uint32_t width = 0;
        uint32_t height = 0;
        hr = MFGetAttributeSize(type.get(), MF_MT_FRAME_SIZE, &width, &height);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFGetAttributeSize(MF_MT_FRAME_SIZE) failed, " << win32::winerr_str(hr);
            continue;
        }

        uint32_t num = 0;
        uint32_t den = 0;
        hr = MFGetAttributeRatio(type.get(), MF_MT_FRAME_RATE, &num, &den);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFGetAttributeSize(MF_MT_FRAME_SIZE) failed, " << win32::winerr_str(hr);
            continue;
        }

        modes.push_back({ pixel_format_from_GUID(format), core::vec2i{ int32_t(width), int32_t(height) }, core::vec2i{ int32_t(num), int32_t(den) } });
    }
    return modes;
}

std::shared_ptr<video_capture_source> create_video_source(std::string deviceid)
{
    HRESULT hr = S_OK;

	win32::com_ptr<IMFAttributes> source_attributes;
    hr = MFCreateAttributes(source_attributes.getpp(), 2);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, core::u8_ucs2(deviceid).c_str());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK failed, " << win32::winerr_str(hr);
        return {};
    }

	win32::com_ptr<IMFMediaSource> source;
    hr = MFCreateDeviceSource(source_attributes.get(), source.getpp());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateDeviceSource failed, " << win32::winerr_str(hr);
        return {};
    }

    video_capture_source_impl * impl = new video_capture_source_impl(source);
    auto capture_source = std::shared_ptr<video_capture_source_impl>(impl, [](video_capture_source_impl * ptr) { if (ptr) ptr->stop(); SafeRelease(&ptr); });
    return capture_source;
}


std::vector<video_capture_device_desc> audio_devices()
{
    HRESULT hr = S_OK;
	win32::com_ptr<IMFAttributes> enumerator_attributes;
    hr = MFCreateAttributes(enumerator_attributes.getpp(), 1);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = enumerator_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        return {};
    }

    IMFActivate ** activates = nullptr;
    UINT32 count = 0;
    std::unique_ptr<void, std::function<void(void *)>> devices_releaser(nullptr, [&](void *)
    {
        if (activates)
        {
            for (UINT32 i = 0; i < count; i++)
                SafeRelease(&activates[i]);
            CoTaskMemFree(activates);
        }
    });

    //std::unique_ptr<void, decltype(void())*> devices_releaser(nullptr, [](){});

    hr = MFEnumDeviceSources(enumerator_attributes.get(), &activates, &count);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFEnumDeviceSources failed, " << win32::winerr_str(hr);
        return {};
    }

    std::vector<video_capture_device_desc> devices;
    for (UINT32 idevice = 0; idevice < count; ++idevice)
    {
        IMFActivate * device = activates[idevice];
        core::temp_ptr<WCHAR, decltype(CoTaskMemFree) *> friendlyName(nullptr, CoTaskMemFree);
        hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, friendlyName.getpp(), NULL);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME) failed at " << idevice << ", " << win32::winerr_str(hr);
            continue;
        }

        core::temp_ptr<WCHAR, decltype(CoTaskMemFree) *> deviceId(nullptr, CoTaskMemFree);
        hr = device->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID, deviceId.getpp(), NULL);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK) failed at " << idevice << ", " << win32::winerr_str(hr);
            continue;
        }

        // 部分设备（BlackMagic 采集卡）MFCreateDeviceSource 可能会失败，这里可以过滤掉
        //{
        //    com_ptr<IMFAttributes> source_attributes;
        //    hr = MFCreateAttributes(source_attributes.getpp(), 2);
        //    if (FAILED(hr))
        //    {
        //        core::logger::war() << __FUNCTION__ " set MFCreateAttributes failed, " << win32::winerr_str(hr);
        //        continue;
        //    }

        //    hr = source_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        //    if (FAILED(hr))
        //    {
        //        core::logger::war() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        //        continue;
        //    }

        //    hr = source_attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, deviceId.get());
        //    if (FAILED(hr))
        //    {
        //        core::logger::war() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK failed, " << win32::winerr_str(hr);
        //        continue;
        //    }

        //    com_ptr<IMFMediaSource> source;
        //    hr = MFCreateDeviceSource(source_attributes.get(), source.getpp());
        //    if (FAILED(hr))
        //    {
        //        core::logger::err() << __FUNCTION__ " MFCreateDeviceSource failed, " << win32::winerr_str(hr);
        //        continue;
        //    }
        //}

        devices.push_back({ core::ucs2_u8(friendlyName.get()), core::ucs2_u8(deviceId.get()) });
    }

    return devices;
}

std::vector<audio_capture_mode> audio_device_modes(std::string deviceid)
{
    HRESULT hr = S_OK;

	win32::com_ptr<IMFAttributes> source_attributes;
    hr = MFCreateAttributes(source_attributes.getpp(), 2);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID, core::u8_ucs2(deviceid).c_str());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK failed, " << win32::winerr_str(hr);
        return {};
    }

	win32::com_ptr<IMFMediaSource> source;
    hr = MFCreateDeviceSource(source_attributes.get(), source.getpp());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateDeviceSource failed, " << win32::winerr_str(hr);
        return {};
    }

	win32::com_ptr<IMFAttributes> reader_attributes;
    hr = MFCreateAttributes(reader_attributes.getpp(), 1);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = reader_attributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, true);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_READWRITE_DISABLE_CONVERTERS failed, " << win32::winerr_str(hr);
        return {};
    }
	win32::com_ptr<IMFSourceReader> reader;
    hr = MFCreateSourceReaderFromMediaSource(source.get(), reader_attributes.get(), reader.getpp());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateSourceReaderFromMediaSource failed, " << win32::winerr_str(hr);
        return {};
    }

    std::vector<audio_capture_mode> modes;
    uint32_t type_index = 0;
    while (true)
    {
		win32::com_ptr<IMFMediaType> type;
        hr = reader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, type_index++, type.getpp());
        if (FAILED(hr))
            break;

        GUID formatGUID = {};
        hr = type->GetGUID(MF_MT_SUBTYPE, &formatGUID);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " get MF_MT_SUBTYPE failed, " << win32::winerr_str(hr);
            continue;
        }

        core::sample_format format = sample_format_from_GUID(formatGUID);
        if (format == core::sample_format::none)
            continue;

        uint32_t samplerate = MFGetAttributeUINT32(type.get(), MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFGetAttributeSize(MF_MT_AUDIO_SAMPLES_PER_SECOND) failed, " << win32::winerr_str(hr);
            continue;
        }

        uint32_t channels = MFGetAttributeUINT32(type.get(), MF_MT_AUDIO_NUM_CHANNELS, 0);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFGetAttributeSize(MF_MT_AUDIO_NUM_CHANNELS) failed, " << win32::winerr_str(hr);
            continue;
        }

        uint32_t bits = MFGetAttributeUINT32(type.get(), MF_MT_AUDIO_BITS_PER_SAMPLE, 0);
        if (FAILED(hr))
        {
            core::logger::war() << __FUNCTION__ " MFGetAttributeSize(MF_MT_AUDIO_NUM_CHANNELS) failed, " << win32::winerr_str(hr);
            continue;
        }

        modes.push_back({ format, (int32_t)samplerate, (int32_t)channels });
    }
    return modes;
}

std::shared_ptr<audio_capture_source> create_audio_source(std::string deviceid)
{
    HRESULT hr = S_OK;

	win32::com_ptr<IMFAttributes> source_attributes;
    hr = MFCreateAttributes(source_attributes.getpp(), 2);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MFCreateAttributes failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE failed, " << win32::winerr_str(hr);
        return {};
    }

    hr = source_attributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID, core::u8_ucs2(deviceid).c_str());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " set MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK failed, " << win32::winerr_str(hr);
        return {};
    }

	win32::com_ptr<IMFMediaSource> source;
    hr = MFCreateDeviceSource(source_attributes.get(), source.getpp());
    if (FAILED(hr))
    {
        core::logger::err() << __FUNCTION__ " MFCreateDeviceSource failed, " << win32::winerr_str(hr);
        return {};
    }

    audio_capture_source_impl * impl = new audio_capture_source_impl(source);
    auto capture_source = std::shared_ptr<audio_capture_source_impl>(impl, [](audio_capture_source_impl * ptr) { if (ptr) ptr->stop(); SafeRelease(&ptr); });
    return capture_source;
}

video_device& video_device::getInstance()
{
	static std::shared_ptr<video_device> __video_device;
	static std::once_flag _video_devic_once_flag;
	std::call_once(_video_devic_once_flag, []() { __video_device = std::make_shared<video_device>(); });
	return *__video_device;
}

void video_device::addCameraDeviceDesc(const video_capture_device_desc& desc)
{
	_cameraList.push_back(desc);
}

std::vector<video_capture_device_desc> video_device::devices() const
{
	return _cameraList;
}

bool video_device::empty() const
{
	return _cameraList.empty();
}

void video_device::clear()
{
	_cameraList.clear();
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
    #include "Windows/HideWindowsPlatformTypes.h"
#endif