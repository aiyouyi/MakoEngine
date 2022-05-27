#include "ThreadPool.h"
#include <windows.h>
#include"RunnableTask.h"

void ThreadPool::ExecuteRunnable(class RunnableTask* Task, bool DefaultDelete, std::function<void(void)> AutoDelete)
{
	auto pThread = GetIdleThread();
	pThread->TaskFunc = [Task, DefaultDelete, AutoDelete]() {
		if (Task->Init())
		{
			Task->Run();
		}
		Task->Exit();
		if (DefaultDelete)
		{
			delete Task;
		}
		else if (AutoDelete)
		{
			AutoDelete();
		}
	};
}

MThread::MThread()
{
	MThread* pThread = this;
	std::thread* pth= new std::thread{
		[pThread]
		{
			while (!pThread->bQuit)
			{
				if (pThread->TaskFunc)
				{
					pThread->bIdle = false;
					pThread->TaskFunc();
					pThread->TaskFunc = nullptr;
				}
				pThread->bIdle = true;
				Sleep(1);
			}
	}
	};
}
