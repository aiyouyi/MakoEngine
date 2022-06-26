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

int Func0()
{
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
		}
		Sleep(1);
		
	} while (MWindow::WindowsNum>0);
	return 0;
}


int Func1()
{
	//GLBMeshPool::GetInstance()->GetGLBMesh(L"G:\\CCFilterTmp\\FacialToolsQT-DX11\\resource\\jiumi01_AR\\gxll_anima.glb");
	GLBMeshPool::GetInstance()->GetGLBMesh(L"G:\\makoengine\\MakoEngine\\Resource\\DamagedHelmet.glb");
	return 0;
}