
#include "common/multimediatimer.h"

#include "win/win32.h"
#include "MMSystem.h"

typedef void(*TIMER_CALLBACK_FUNCTION)(LPVOID instance);

static void CALLBACK timerEventProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(dw1);
	UNREFERENCED_PARAMETER(dw2);

	CMultimediaTimer *pThis = (CMultimediaTimer*)(dwUser);
	if (pThis)
	{
		pThis->innerTimerEventProc(uTimerID);
	}
}
CMultimediaTimer* getMultimediaTimer()
{
	return new CMultimediaTimer();
}

CMultimediaTimer::CMultimediaTimer()
: m_timerId(0)
{

}

CMultimediaTimer::~CMultimediaTimer()
{
	if (m_timerId)
	{
		timeKillEvent(m_timerId);
		m_timerId = 0;
	}
}

void CMultimediaTimer::release()
{
	delete this;
}

bool CMultimediaTimer::start(uint32 uDelay)
{
	if (m_timerId)
	{
		return true;
	}
	m_timerId = ::timeSetEvent(uDelay, 0, &timerEventProc, (DWORD_PTR)this,TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
	return m_timerId != NULL;
}

void CMultimediaTimer::stop(void)
{
	if (m_timerId)
	{
		::timeKillEvent(m_timerId);
		m_timerId = 0;
	}
}

bool CMultimediaTimer::isStarted()
{
	return m_timerId != 0;
}


void CMultimediaTimer::innerTimerEventProc(uint32 uTimerID)
{
	if (m_timerId == uTimerID)
	{
		sigTimeout();
	}
}

#ifdef WINDOWS_PLATFORM_TYPES_GUARD
#include "Windows/HideWindowsPlatformTypes.h"
#endif