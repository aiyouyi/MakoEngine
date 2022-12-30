#pragma once

#include "core/inc.h"
#include "core/event.h"
#include <Runtime/Core/Public/CoreMinimal.h>

namespace core
{
	class CCACTORLIB_API timer
	{
	public:
        timer();
		timer(std::chrono::milliseconds period);
		~timer();

		timer(const timer & another) = delete;

		void start();
		void start(std::chrono::milliseconds period);
		void stop();

		void callback();

	public:
        core::event<void(timer &, int64_t)> tick;

	private:
        std::shared_ptr<void> _context;
		std::atomic<bool> _started = false;
		std::chrono::milliseconds _period = 1000ms;
		int64_t _tick = 0;
	};

	CCACTORLIB_API bool os_sleepto_ns(uint64_t time_target);
	CCACTORLIB_API void os_sleep_ms(uint32_t duration);

	CCACTORLIB_API uint64_t os_gettime_ns(void);

}
