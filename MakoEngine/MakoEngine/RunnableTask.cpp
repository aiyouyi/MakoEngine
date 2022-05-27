#include "RunnableTask.h"
#include"ThreadPool.h"



RunnableTask::RunnableTask()
{
}

void RunnableTask::Execute()
{
	ThreadPool::GetInstance().ExecuteRunnable(this);
}
