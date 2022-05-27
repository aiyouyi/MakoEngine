#pragma once
#include<functional>
#include<thread>
#include<list>

class MThread
{
public:
	MThread();
	bool bIdle = true;
	bool bQuit = false;
	std::function<void(void)> TaskFunc;
};

class ThreadPool
{
public:
	static ThreadPool& GetInstance()
	{
		static ThreadPool Ins;
		return Ins;
	}

	virtual ~ThreadPool()
	{
		for (MThread* pThread : AllThreads)
		{
			pThread->bQuit = true;
		}
	}
	void ExecuteRunnable(class RunnableTask* Task, bool DefaultDelete=true, std::function<void(void)> AutoDelete = nullptr);

private:
	MThread* GetIdleThread()
	{
		for (MThread * pThread : AllThreads)
		{
			if (pThread&& pThread->bIdle)
			{
				return pThread;
			}
		}
		return AllocateMoreThread();
	}

	MThread* AllocateMoreThread()
	{
		MThread* pMThread = new MThread();
		AllThreads.push_back(pMThread);
		ThreadNum++;
		return pMThread;
	} 
	std::list<MThread*> AllThreads;
	int ThreadNum = 0;
};

