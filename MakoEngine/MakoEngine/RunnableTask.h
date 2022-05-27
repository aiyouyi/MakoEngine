#pragma once
class RunnableTask
{
public:
	RunnableTask();
	void Execute();
	virtual bool Init() { return true; };
	virtual void Run() = 0;
	virtual void Exit() {};
};