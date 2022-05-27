#include"MWindow.h"

int main()
{
	MWindow Window;
	Window.CreateWindows();
	MSG msg = {};
	do
	{
		bool GotMessage = ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (GotMessage)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		Sleep(1);
		
	} while (MWindow::WindowsNum>0);
	return 0;
}