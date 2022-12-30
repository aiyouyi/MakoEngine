#include "hook/dxhook.h"
#include "dx11hook.h"

DXHook::DXHook()
{
	bReceiveSharedTexHandle = false;
	sharedTexHandle = NULL;
}

DXHook::~DXHook()
{
	sharedTexHandle = NULL;
}

DXHook& DXHook::getInstance()
{
	static DXHook hook;
	return hook;
}

bool DXHook::hookFunction(bool canUseSharedCopy)
{
	CDX11Hook::getInstance().sigGpuImageCapture += std::bind(&DXHook::onSigGpuImageCapture, this);
	CDX11Hook::getInstance().sigCpuImageCapture += std::bind(&DXHook::onCpuImageCapture, this, std::placeholders::_1, std::placeholders::_2);
	CDX11Hook::getInstance().sigSendUseSharedTexCapture += std::bind(&DXHook::onSigSendUseSharedTexCapture, this, std::placeholders::_1);

	CDX11Hook::getInstance().sigSetSharedViewHandle += std::bind(&DXHook::onsigSetSharedViewHandle, this, std::placeholders::_1);

	return CDX11Hook::getInstance().hookFunction(canUseSharedCopy);
}

void DXHook::doCpuImageCapture()
{
	CDX11Hook::getInstance().doCpuImageCapture();
}

HANDLE DXHook::getSharedTexHandle()
{
	return sharedTexHandle;
}

bool DXHook::getReceiveSharedTexHandle()
{
	return bReceiveSharedTexHandle;
}

void DXHook::setReceiveSharedTexHandle(bool bParam)
{
	bReceiveSharedTexHandle = bParam;
}

void DXHook::SetCpuHook(bool use)
{
	CDX11Hook::getInstance().setCpuHook(use);
}

void DXHook::SetMainWindow(void* hWnd)
{
	CDX11Hook::getInstance().SetMainWindow(hWnd);
}

void DXHook::onSigGpuImageCapture()
{
	//sigGpuImageCapture();
}

void DXHook::onCpuImageCapture(byte_t * color, core::vec2i viewport)
{
	//sigCpuImageCapture(color, viewport);
}

void DXHook::onSigSendUseSharedTexCapture(const std::string& name)
{
	//sigSendUseSharedTexCapture(name);
}

void DXHook::onsigSetSharedViewHandle(const HANDLE handle)
{
	bReceiveSharedTexHandle = true;
	sharedTexHandle = handle;
	//sigSetSharedViewHandle(handle);

	if (CCActorlibSharedViewHandle.IsBound())
	{
		CCActorlibSharedViewHandle.Execute(true);
	}
}

