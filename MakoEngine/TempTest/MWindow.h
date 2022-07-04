#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include<string>
#include<map>
#include<functional>

class MWindow
{
public:
	static int WindowsNum;
	MWindow();
	virtual ~MWindow();

	void CreateWindows();
	virtual LRESULT WindowProcEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	int Width = 1280;
	int Height = 720;
	std::wstring Title;
	HWND WindowHwnd;
	std::function<LRESULT(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)> InputProcessFunc;
private:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
};

