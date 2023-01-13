#pragma once

#include <windows.h>

namespace SYSTEMSTATUS
{

class CSLock
{
public:
	CSLock()
	{
		::InitializeCriticalSection(&m_cs);
	}
	~CSLock()
	{
		::DeleteCriticalSection(&m_cs);
	}
	void Lock()
	{
		::EnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}
protected:
	CRITICAL_SECTION m_cs;
};

class CSAutoLock
{
public:
	CSAutoLock(CSLock& lock)
	{
		m_lock = &lock;
		m_lock->Lock();
	}
	~CSAutoLock()
	{
		m_lock->Unlock();
	}
protected:
	CSLock* m_lock;
};

}