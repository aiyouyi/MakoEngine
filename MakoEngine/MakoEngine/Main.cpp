#include"MWindow.h"
#include"GLBMeshData.h"
#include"FGame.h"

#define STest 0 
#if STest
#define Func0 main2
#define Func1 main
#else
#define Func0 main
#define Func1 main2
#endif // DEBUG

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

int Func0()
{
	//Initialize();
	MWindow Window;
	Window.CreateWindows();
	FGame Game(&Window);
	MSG msg = {};
	do
	{
		bool GotMessage = ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		if (GotMessage)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			Game.Tick();
			Sleep(100);
		}
		
		
	} while (MWindow::WindowsNum>0);
	return 0;
}


int Func1()
{
	return 0;
}