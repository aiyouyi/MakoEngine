#pragma once
#include <Runtime/Core/Public/CoreMinimal.h>
#include "core/event.h"

class  CCACTORLIB_API CMultimediaTimer
{
public:
	CMultimediaTimer();
	~CMultimediaTimer();

	void release();
	bool start(uint32 uDelay);
	void stop();
	bool isStarted();

	// Event
	core::event<void() >    sigTimeout;
	
	void innerTimerEventProc(uint32 uTimerID);

private:
	uint32 m_timerId;
};

CMultimediaTimer* getMultimediaTimer();


