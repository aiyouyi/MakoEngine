#pragma once
#include"MWindow.h"


class FGame
{
public:
	FGame(MWindow* pMWindow)
	{
		pWindow = pMWindow;
	}
	MWindow* pWindow;
};

