#include "core/timer.h"
#include "win/win32.h"

namespace core
{
    static void CALLBACK TimerCallBack(HWND, UINT, UINT_PTR timerId, DWORD)
    {
        timer * ptimer = reinterpret_cast<timer *>(timerId);
        ptimer->callback();
    }

    const wchar_t _timerHWND_ClassName[] = L"{2A84CC0A-7D4F-42AB-9311-DEA336A1F289}";
    static thread_local std::weak_ptr<struct TimerContext> __timerContext;
    struct TimerContext
    {
        std::weak_ptr<struct TimerContext> & _timerContext = __timerContext;
        std::mutex _mtx;
       HWND _timerHWND = NULL;
       bool _timerHWND_Registed = false;
       int _timerHWND_RefCount = 0;

       void SetTimer(timer * ptimer, std::chrono::milliseconds period)
       {
           std::lock_guard<std::mutex> lock(_mtx);
           if(!ptimer)
               return;

           if(!_timerHWND)
           {
               HINSTANCE hInstance = GetModuleHandle(NULL);
               if(!_timerHWND_Registed)
               {
                   WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
                   wcex.style = CS_HREDRAW | CS_VREDRAW;
                   wcex.lpfnWndProc = DefWindowProcW;
                   wcex.cbClsExtra = 0;
                   wcex.cbWndExtra = 0;
                   wcex.hInstance = hInstance;
                   wcex.hIcon = NULL;
                   wcex.hCursor = NULL;
                   wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
                   wcex.lpszMenuName = NULL;
                   wcex.lpszClassName = _timerHWND_ClassName;
                   wcex.hIconSm = NULL;

                   RegisterClassExW(&wcex);
               }
               _timerHWND = CreateWindowExW(0, _timerHWND_ClassName, L"_timerHWND", WS_OVERLAPPEDWINDOW,
                   0, 0, CW_DEFAULT, CW_DEFAULT, NULL, NULL, hInstance, NULL);
           }

           ::SetTimer(_timerHWND, (UINT_PTR)(void *)ptimer, (UINT)period.count(), TimerCallBack);
           ++_timerHWND_RefCount;
       }

       static std::shared_ptr<TimerContext> GetTimerContext()
       {
           std::shared_ptr<TimerContext> timerContext = __timerContext.lock();
           if(!timerContext)
           {
               timerContext = std::make_shared<TimerContext>();
               __timerContext = timerContext;
           }
           return timerContext;
       }

       void KillTimer(timer * ptimer)
       {
           std::lock_guard<std::mutex> lock(_mtx);
           if(!ptimer || !_timerHWND || !_timerHWND_RefCount)
               return;

           ::KillTimer(_timerHWND, (UINT_PTR)(void *)ptimer);
           --_timerHWND_RefCount;
           if(!_timerHWND_RefCount)
           {
               ::DestroyWindow(_timerHWND);
               _timerHWND = nullptr;
               _timerContext.reset();
           }
       }
    };


    timer::timer():_context(TimerContext::GetTimerContext())
    {
	    
    }

	timer::timer(std::chrono::milliseconds period): _context(TimerContext::GetTimerContext()), _period(period)
	{

	}

	timer::~timer()
	{
        bool _true = true;
		if(_started.compare_exchange_strong(_true, false))
        {
            std::shared_ptr<TimerContext> timerContext = std::reinterpret_pointer_cast<TimerContext, void>(_context);
            timerContext->KillTimer(this);
            _context.reset();
		}
	}

	void timer::start()
	{
        if(!_context)
            return;

        bool _false = false;
		if(!_started.compare_exchange_strong(_false, true))
			return;

        std::shared_ptr<TimerContext> timerContext = std::reinterpret_pointer_cast<TimerContext, void>(_context);
        timerContext->SetTimer(this, _period);
	}

	void timer::start(std::chrono::milliseconds period)
	{
        if(!_context)
            return;

        bool _false = false;
        if(!_started.compare_exchange_strong(_false, true))
            return;

        std::shared_ptr<TimerContext> timerContext = std::reinterpret_pointer_cast<TimerContext, void>(_context);
        _period = period;
        timerContext->SetTimer(this, _period);
	}

	void timer::stop()
	{
        if(!_context)
            return;

        bool _true = true;
        if(!_started.compare_exchange_strong(_true, false))
            return;

        std::shared_ptr<TimerContext> timerContext = std::reinterpret_pointer_cast<TimerContext, void>(_context);
        timerContext->KillTimer(this);
        _started = false;
        _tick = 0;
	}

	void timer::callback()
	{
        tick(std::ref(*this), _tick++);
	}

	static bool have_clockfreq = false;
	static LARGE_INTEGER clock_freq;

	static inline uint64_t get_clockfreq(void)
	{
		if (!have_clockfreq) {
			QueryPerformanceFrequency(&clock_freq);
			have_clockfreq = true;
		}

		return clock_freq.QuadPart;
	}


    CCACTORLIB_API bool os_sleepto_ns(uint64_t time_target)
    {
		uint64_t t = os_gettime_ns();
		uint32_t milliseconds;

		if (t >= time_target)
			return false;

		milliseconds = (uint32_t)((time_target - t) / 1000000);
		if (milliseconds > 1)
			Sleep(milliseconds - 1);

		for (;;) {
			t = os_gettime_ns();
			if (t >= time_target)
				return true;

#if 0
			Sleep(1);
#else
			Sleep(0);
#endif
		}
    }
    CCACTORLIB_API void os_sleep_ms(uint32_t duration)
    {
		/* windows 8+ appears to have decreased sleep precision */
		if (win32::version() >= win32::winversion_8  && duration > 0)
			duration--;

		Sleep(duration);
    }

    CCACTORLIB_API uint64_t os_gettime_ns(void)
    {
		LARGE_INTEGER current_time;
		double time_val;

		QueryPerformanceCounter(&current_time);
		time_val = (double)current_time.QuadPart;
		time_val *= 1000000000.0;
		time_val /= (double)get_clockfreq();

		return (uint64_t)time_val;
    }
}
#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif
