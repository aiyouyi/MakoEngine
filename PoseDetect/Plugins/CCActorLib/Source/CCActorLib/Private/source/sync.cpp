#include "win/sync.h"
#include "win/win32.h"

namespace win32
{
    signal::signal()
    {

    }

    signal::signal(bool init, bool manual)
    {
        create(std::string(), init, manual);
    }

    signal::signal(std::string name, bool init, bool manual)
    {
        create(name, init, manual);
    }

    signal::signal(signal && another) noexcept
    {
        _handle = another._handle;
        another._handle = nullptr;
    }

    signal::~signal()
    {
        if(_handle)
            CloseHandle((HANDLE)_handle);
    }

    core::error_e signal::create(std::string name, bool init, bool manual)
    {
        core::bitflag<uint32_t> flags = nullptr;
        flags.set(CREATE_EVENT_INITIAL_SET, init);
        flags.set(CREATE_EVENT_MANUAL_RESET, manual);
        _handle = CreateEventExW(NULL, core::u8_ucs2(name).c_str(), flags, SYNCHRONIZE | EVENT_MODIFY_STATE);
        if (!_handle)
            return win32::winerr();
        return core::error_ok;
    }

    core::error_e signal::open(std::string name)
    {
        _handle = OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, false, core::u8_ucs2(name).c_str());
        if (!_handle)
            return win32::winerr();
        return core::error_ok;
    }

    core::error_e signal::wait()
    {
        auto wait = WaitForSingleObject((HANDLE)_handle, INFINITE);
        switch (wait)
        {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            return core::error_ok;
        case WAIT_TIMEOUT:
            return core::error_timeout;
        case WAIT_FAILED:
            return win32::winerr();
        default:
            return core::error_generic;
        }
    }

    core::error_e signal::wait_for(std::chrono::milliseconds timeout)
    {
        auto wait = WaitForSingleObject((HANDLE)_handle, (DWORD)timeout.count());
        switch (wait)
        {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            return core::error_ok;
        case WAIT_TIMEOUT:
            return core::error_timeout;
        case WAIT_FAILED:
            return win32::winerr();
        default:
            return core::error_generic;
        }
    }

    core::error_e signal::set()
    {
        if (!SetEvent((HANDLE)_handle))
            return win32::winerr();
        else
            return core::error_ok;
    }

    core::error_e signal::reset()
    {
        if (!ResetEvent((HANDLE)_handle))
            return win32::winerr();
        else
            return core::error_ok;
    }

    signal & signal::operator = (signal && another) noexcept
    {
        _handle = another._handle;
        another._handle = nullptr;
        return *this;
    }

    mutex::mutex()
    {

    }

    mutex::mutex(bool own)
    {
        _handle = CreateMutexW(NULL, BOOL(own), NULL);
    }

    mutex::mutex(std::string name, bool own)
    {
        auto wname = core::u8_ucs2(name);
        _handle = CreateMutexW(NULL, BOOL(own), wname.c_str());
    }

    mutex::mutex(mutex && another) noexcept
    {
        _handle = another._handle;
        another._handle = nullptr;
    }

    mutex::~mutex()
    {
        CloseHandle((HANDLE)_handle);
    }

    core::error_e mutex::create(std::string name, bool own)
    {
        core::bitflag<uint32_t> flags = nullptr;
        flags.set(CREATE_MUTEX_INITIAL_OWNER, own);
        auto wname = core::u8_ucs2(name);
        _handle = CreateMutexExW(NULL, wname.c_str(), flags, SYNCHRONIZE);
        if (!_handle)
            return win32::winerr();
        return core::error_ok;
    }

    core::error_e mutex::open(std::string name)
    {
        auto namew = core::u8_ucs2(name);
        _handle = OpenMutexW(SYNCHRONIZE, false, namew.c_str());
        if(!_handle)
            return win32::winerr();
        return core::error_ok;
    }

    core::error_e mutex::lock()
    {
        auto wait = WaitForSingleObject((HANDLE)_handle, INFINITE);
        switch (wait)
        {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            return core::error_ok;
        case WAIT_TIMEOUT:
            return core::error_timeout;
        case WAIT_FAILED:
            return win32::winerr();
        default:
            return core::error_generic;
        }
    }

    core::error_e mutex::try_lock(std::chrono::milliseconds timeout)
    {
        auto wait = WaitForSingleObject((HANDLE)_handle, (DWORD)timeout.count());
        switch (wait)
        {
        case WAIT_OBJECT_0:
        case WAIT_ABANDONED:
            return core::error_ok;
        case WAIT_TIMEOUT:
            return core::error_timeout;
        case WAIT_FAILED:
            return win32::winerr();
        default:
            return core::error_generic;
        }
    }

    core::error_e mutex::unlock()
    {
        if (!ReleaseMutex((HANDLE)_handle))
            return win32::winerr();
        else
            return core::error_ok;
    }

    mutex & mutex::operator = (mutex && another) noexcept
    {
        _handle = another._handle;
        another._handle = nullptr;
        return *this;
    }
}
#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif