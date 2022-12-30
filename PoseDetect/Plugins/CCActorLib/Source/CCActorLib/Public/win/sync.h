#pragma once
#include "CoreMinimal.h"
#include "core/inc.h"

namespace win32
{
    class CCACTORLIB_API signal
    {
    public:
        signal();
        signal(bool init, bool manual);
        signal(std::string name, bool init, bool manual);
        signal(signal && another) noexcept;
        ~signal();

        core::error_e create(std::string name, bool init, bool manual);
        core::error_e open(std::string name);

        core::error_e wait();
        core::error_e wait_for(std::chrono::milliseconds timeout);
        core::error_e set();
        core::error_e reset();

        void * handle() const { return _handle; }

        signal & operator =(signal && another) noexcept;
    private:
        void * _handle = nullptr;
    };

    class CCACTORLIB_API mutex
    {
    public:
        mutex();
        mutex(bool own);
        mutex(std::string name, bool own);
        mutex(mutex && another) noexcept;
        ~mutex();

        core::error_e create(std::string name, bool own);
        core::error_e open(std::string name);

        core::error_e lock();
        core::error_e try_lock(std::chrono::milliseconds timeout);
        core::error_e unlock();

        void * handle() const { return _handle; }

        mutex & operator =(mutex && another) noexcept;
    private:
        void * _handle = nullptr;
    };
}
