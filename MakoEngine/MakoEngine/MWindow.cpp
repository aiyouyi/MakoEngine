#include "MWindow.h"
#include <windowsx.h>
#include <algorithm>



const TCHAR AppWindowClass[] = L"MakoWindow";
int MWindow::WindowsNum = 0;

MWindow::MWindow()
{
	//Title = L"MakoWindow";
}

MWindow::~MWindow()
{
}

void MWindow::CreateWindows()
{
	HINSTANCE InstanceHandle = GetModuleHandle(nullptr);
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = InstanceHandle;
	wc.lpszClassName = AppWindowClass;

	int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
	int windowX = std::max<int>(0, (screenWidth - Width) / 2);
	int windowY = std::max<int>(0, (screenHeight - Height) / 2);

	RegisterClassEx(&wc);

	RECT windowRect = { 0, 0, static_cast<LONG>(Width), static_cast<LONG>(Height) };
	AdjustWindowRect(&windowRect, WS_POPUP | WS_SYSMENU | WS_SIZEBOX, FALSE);

	WindowHwnd = CreateWindowEx(
		0,									// Optional window styles
		AppWindowClass,						// Window class
		Title.c_str(),		// Window title
		WS_POPUP | WS_SYSMENU | WS_SIZEBOX,				// Window style
		windowX, windowY,					// Window Position
		windowRect.right - windowRect.left,	// Width
		windowRect.bottom - windowRect.top,	// Height
		NULL,								// Parent window
		NULL,								// Menu
		InstanceHandle,						// Instance handle
		this								// Additional application data
	);
	
	::ShowWindow(WindowHwnd, SW_SHOWNORMAL);
	MWindow::WindowsNum++;
}

LRESULT MWindow::WindowProcEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (InputProcessFunc)
	{
		return InputProcessFunc(hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
		}
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		SetCapture(hWnd);
		break;
	}

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	{
		ReleaseCapture();
		break;
	}
	case WM_DESTROY:
		MWindow::WindowsNum--;
		break;
	default:
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT MWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MWindow* pWindow = nullptr;
	if (uMsg == WM_CREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		pWindow = reinterpret_cast<MWindow*>(pCreateStruct->lpCreateParams);
	}
	else
	{
		pWindow = reinterpret_cast<MWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}
	if (pWindow)
	{
		return pWindow->WindowProcEvent(hWnd, uMsg, wParam, lParam);
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
